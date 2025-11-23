# algorithms.py

import numpy as np
from itertools import combinations, permutations, chain
from pulp import (
    LpProblem, LpVariable, LpBinary, lpSum, LpMinimize,
    value, LpStatusOptimal
)
from math import inf
import random

# ----------------------------------------------------------
# 1. Kendall-tau distance
# ----------------------------------------------------------
def kendall_tau_distance(a: np.ndarray, b: np.ndarray) -> int:
    """
    Returns the (unnormalized) Kendall-tau distance between two rankings.
    """
    n = len(a)
    dist = 0
    for i, j in combinations(range(n), 2):
        if (a[i] - a[j]) * (b[i] - b[j]) < 0:
            dist += 1
    return dist


# ----------------------------------------------------------
# 2. Exact Kemeny-Young using ILP (PuLP)
# ----------------------------------------------------------
def kemeny_young_exact(ranks: np.ndarray):
    n_voters, n_candidates = ranks.shape

    # cost[i,j] = number of voters who prefer j > i (i.e. rank j better than i)
    cost = np.zeros((n_candidates, n_candidates), dtype=int)
    for i in range(n_candidates):
        for j in range(n_candidates):
            if i != j:
                cost[i,j] = np.sum(ranks[:, j] < ranks[:, i])

    prob = LpProblem("Kemeny_Young", LpMinimize)

    # x[i,j] = 1 if we rank i BEFORE j in the consensus (i beats j)
    x = [[LpVariable(f"x{i}_{j}", cat=LpBinary) if i != j else None
          for j in range(n_candidates)] for i in range(n_candidates)]

    # Objective
    prob += lpSum(cost[i][j] * x[i][j] for i in range(n_candidates) for j in range(n_candidates) if i != j)

    # Antisymmetry
    for i in range(n_candidates):
        for j in range(n_candidates):
            if i != j:
                prob += x[i][j] + x[j][i] == 1

    # Transitivity
    for i in range(n_candidates):
        for j in range(n_candidates):
            for k in range(n_candidates):
                if len({i,j,k}) == 3:
                    prob += x[i][j] + x[j][k] + x[k][i] <= 2

    # Solve (suppress log with msg=False if you want silence)
    prob.solve()   # or: prob.solve(PULP_CBC_CMD(msg=False))

    if prob.status != LpStatusOptimal:
        raise ValueError("No optimal solution found")

    # Extract the transitive tournament
    beats_count = np.zeros(n_candidates)
    for i in range(n_candidates):
        for j in range(n_candidates):
            if i != j and value(x[i][j]) > 0.9:
                beats_count[i] += 1

    # Convert to rank positions (0 = best)
    consensus_ranks = np.zeros(n_candidates, dtype=int)
    order = np.argsort(-beats_count)               # candidates from best to worst
    consensus_ranks[order] = np.arange(n_candidates)  # assign positions 0,1,2,...

    # Now compute the correct Kemeny score
    true_score = sum(kendall_tau_distance(consensus_ranks, voter) for voter in ranks)

    return int(prob.objective.value()), order, true_score
    # 'order' is the sequence of candidate indices from best to worst, true_score


# ----------------------------------------------------------
# 3. 2-approximation algorithms
# ----------------------------------------------------------
def borda(ranks: np.ndarray) -> np.ndarray:
    """
    Borda count approximation: lower total rank = better.
    """
    scores = np.sum(ranks, axis=0)
    return np.argsort(scores)


def copeland(ranks: np.ndarray) -> np.ndarray:
    """
    Copeland score = # of majority pairwise wins.
    """
    n_voters, n_cand = ranks.shape
    wins = np.zeros(n_cand)

    for i in range(n_cand):
        for j in range(n_cand):
            if i != j:
                if np.sum(ranks[:, i] < ranks[:, j]) > n_voters / 2:
                    wins[i] += 1

    return np.argsort(-wins)

# ----------------------------------------------------------
# 8. Brute Force Kemeny (Ranky / Vlad Niculae)
# ----------------------------------------------------------
def kemeny_bruteforce(ranks):
    """
    Brute-force exact Kemeny consensus.

    Input:
        ranks[v, c] = position of candidate c for voter v  (0 = best)
    Output:
        (best_perm, best_score)
    """
    n_voters, m = ranks.shape

    # Precompute pairwise disagreement penalty:
    # cost[a, b] = voters who prefer b > a
    cost = np.zeros((m, m), dtype=int)
    for a in range(m):
        for b in range(m):
            if a != b:
                cost[a, b] = np.sum(ranks[:, a] > ranks[:, b])

    best_score = float('inf')
    best_perm = None

    # Evaluate every permutation
    for perm in permutations(range(m)):   # perm is candidate order
        score = 0

        # Score Kemeny objective: sum cost[a,b] for a before b
        for i in range(m):
            for j in range(i+1, m):
                a = perm[i]
                b = perm[j]
                score += cost[a, b]

        if score < best_score:
            best_score = score
            best_perm = perm

    return list(best_perm)


# ----------------------------------------------------------
# 9. Plackett-Luce (choix library)
# ----------------------------------------------------------
def plackett_luce_mle(ranks):
    import choix

    comps = []
    for ranking in ranks:
        order = np.argsort(ranking)
        for i in range(len(order)):
            for j in range(i + 1, len(order)):
                comps.append((order[i], order[j]))

    params = choix.ilsr_pairwise(len(ranks[0]), comps, alpha=0.01)
    if params is None:
        return None, None   # MLE does not exist

    return np.argsort(-params), params

# ----------------------------------------------------------
# ----------------------------------------------------------
def majority_sort(ranks):
    n = ranks.shape[1]

    def majority_pref(a, b):
        return np.sum(ranks[:, a] < ranks[:, b]) > np.sum(ranks[:, b] < ranks[:, a])

    arr = list(range(n))
    for i in range(n - 1):
        for j in range(n - i - 1):
            if not majority_pref(arr[j], arr[j + 1]):
                arr[j], arr[j + 1] = arr[j + 1], arr[j]
    return arr

# ----------------------------------------------------------
# 5. Footrule Optimal Ranking (Spearman Footrule)
# ----------------------------------------------------------
def footrule_optimal(ranks):
    scores = np.sum(ranks, axis=0)  # equivalent to median for strict rankings
    return np.argsort(scores)


def ranked_pairs(ranks: np.ndarray) -> np.ndarray:
    """
    Ranked Pairs (Tideman) method - excellent Kemeny approximation
    Input: ranks - (voters, candidates), lower number = better rank
    Output: consensus order (best to worst) as array of candidate indices
    """
    voters, cand = ranks.shape
    # Pairwise majority matrix: wins[i,j] = how many voters prefer i > j
    wins = np.zeros((cand, cand), dtype=int)
    for i in range(cand):
        for j in range(cand):
            if i != j:
                wins[i,j] = np.sum(ranks[:,i] < ranks[:,j])

    # List of all possible pairs sorted by strength (descending)
    pairs = []
    for i in range(cand):
        for j in range(cand):
            if i != j:
                strength = abs(wins[i,j] - wins[j,i])
                winner = i if wins[i,j] > wins[j,i] else j
                loser = j if winner == i else i
                pairs.append((strength, winner, loser))

    pairs.sort(reverse=True)  # strongest first

    # Build graph and lock edges
    graph = np.zeros((cand, cand), dtype=bool)  # graph[i,j] = i beats j (locked)
    for strength, winner, loser in pairs:
        if not has_cycle(graph, winner, loser, cand):
            graph[winner, loser] = True

    # Topological sort on the final graph
    indegree = np.sum(graph, axis=0)
    order = []
    queue = [i for i in range(cand) if indegree[i] == 0]
    while queue:
        node = queue.pop(0)
        order.append(node)
        for loser in range(cand):
            if graph[node, loser]:
                indegree[loser] -= 1
                if indegree[loser] == 0:
                    queue.append(loser)
    return np.array(order)

def has_cycle(graph, u, v, n):
    """Simple DFS cycle detection for adding edge u → v"""
    visited = [False] * n
    rec_stack = [False] * n

    def dfs(node):
        visited[node] = True
        rec_stack[node] = True
        for neigh in range(n):
            if graph[node, neigh]:
                if not visited[neigh]:
                    if dfs(neigh):
                        return True
                elif rec_stack[neigh]:
                    return True
        rec_stack[node] = False
        return False

    # Temporarily add edge
    graph[u,v] = True
    cycle = dfs(v)  # if path back to v → cycle
    graph[u,v] = False  # revert
    return cycle

def kemeny_local_search(ranks, max_restarts=20, max_iters_no_improve=2000):
    n_voters, m = ranks.shape

    # Precompute pairwise penalties P[a,b] = voters preferring b>a
    P = np.zeros((m, m), dtype=int)
    for a in range(m):
        for b in range(m):
            if a != b:
                P[a, b] = np.sum(ranks[:, b] < ranks[:, a])

    def score_from_order(order):
        score = 0
        for i in range(m):
            for j in range(i+1, m):
                score += P[order[i], order[j]]
        return score

    best_score = float('inf')
    best_order = None

    for r in range(max_restarts):
        # Borda start
        initial = np.sum(ranks, axis=0)
        order = np.argsort(initial)
        current_score = score_from_order(order)

        iters_no_improve = 0

        while iters_no_improve < max_iters_no_improve:
            improved = False
            for i in range(m - 1):
                a, b = order[i], order[i+1]

                # delta = what happens if we swap a and b
                # removing P[a,b], adding P[b,a]
                delta = P[b, a] - P[a, b]
                new_score = current_score + delta

                if new_score < current_score:
                    # swap permanently
                    order[i], order[i+1] = b, a
                    current_score = new_score
                    improved = True
                    iters_no_improve = 0
                else:
                    iters_no_improve += 1

            if not improved:
                break

        if current_score < best_score:
            best_score = current_score
            best_order = order.copy()

    return best_score, best_order


def random_ranking(ranks: np.ndarray):
    order = np.random.permutation(ranks.shape[1])
    return order

def schulze(ranks: np.ndarray) -> np.ndarray:
    """
    Schulze method (Beatpath) – excellent Kemeny approximation
    Input: ranks – (voters, candidates), lower number = better
    Output: consensus order (best → worst)
    """
    n_voters, n_cand = ranks.shape

    # d[i,j] = number of voters who prefer i over j
    d = np.zeros((n_cand, n_cand), dtype=int)
    for i in range(n_cand):
        for j in range(n_cand):
            if i != j:
                d[i,j] = np.sum(ranks[:,i] < ranks[:,j])

    # Floyd-Warshall for strongest paths
    p = d.copy()
    for i in range(n_cand):
        for j in range(n_cand):
            if i != j:
                for k in range(n_cand):
                    if i != k and j != k:
                        p[j,k] = max(p[j,k], min(p[j,i], p[i,k]))

    # Count how many candidates each beats on strongest path
    wins = np.sum(p > p.T, axis=1)
    order = np.argsort(-wins)   # higher wins = better rank

    return order


def compute_pairwise_cost(ranks: np.ndarray) -> np.ndarray:
    """
    ranks: shape (n_voters, m_candidates), lower rank = better (0 is top)
    cost[a, b] = #voters who prefer b over a
                 = penalty we pay if we put a ABOVE b in the final ranking.
    """
    n_voters, m = ranks.shape
    cost = np.zeros((m, m), dtype=int)
    for a in range(m):
        for b in range(m):
            if a == b:
                continue
            # voter prefers b over a if rank[b] < rank[a]
            cost[a, b] = np.sum(ranks[:, b] < ranks[:, a])
    return cost


def kemeny_dp_by_candidates(ranks: np.ndarray):
    """
    Exact Kemeny consensus using DP over subsets.
    Parameter = number of candidates m.
    Running time: O(2^m * m^2 * n).

    Input:
        ranks[v, c] = position of candidate c in voter v's ranking
                      (0 = best, higher = worse)

    Output:
        (opt_score, order)
        opt_score: optimal Kemeny score (integer)
        order: list of candidate indices from best to worst
    """
    n_voters, m = ranks.shape
    if m == 0:
        return 0, []
    if m == 1:
        return 0, [0]

    cost = compute_pairwise_cost(ranks)

    # dp[S] = minimum cost for ranking the subset S (bitmask) of candidates
    # last[S] = candidate index that is placed LAST in the optimal ranking of S
    size = 1 << m
    dp = [inf] * size
    last = [-1] * size

    # base cases: singleton subsets
    for i in range(m):
        S = 1 << i
        dp[S] = 0
        last[S] = i

    # iterate over all subsets
    for S in range(1, size):
        # skip singletons (already initialised)
        if S & (S - 1) == 0:
            continue

        # try each candidate i as the LAST element in S
        # i must be in S
        subset_candidates = [i for i in range(m) if (S & (1 << i))]
        for i in subset_candidates:
            prevS = S ^ (1 << i)   # S without i

            # cost of best ranking for prevS
            prev_cost = dp[prevS]
            if prev_cost is inf:
                continue

            # extra penalty from placing i at the bottom of S:
            # everyone in prevS is above i, so for each c in prevS
            # we pay cost[c, i] (penalty of c above i).
            extra = 0
            for c in range(m):
                if prevS & (1 << c):
                    extra += cost[c, i]

            cur = prev_cost + extra
            if cur < dp[S]:
                dp[S] = cur
                last[S] = i

    full = (1 << m) - 1
    opt_score = dp[full]
    if opt_score is inf:
        raise RuntimeError("DP failed to compute a finite Kemeny score")

    # reconstruct ranking from last[] table
    order_reversed = []
    S = full
    while S:
        i = last[S]
        order_reversed.append(i)
        S ^= (1 << i)

    # we built it from worst → best, so reverse
    order = list(reversed(order_reversed))

    return int(opt_score), order

def compute_majority_matrix(ranks):
    n_voters, m = ranks.shape
    M = np.zeros((m, m), dtype=int)
    for a in range(m):
        for b in range(m):
            if a != b:
                # 1 if a beats b
                M[a, b] = np.sum(ranks[:, a] < ranks[:, b])
                M[a, b] = 1 if M[a, b] > (n_voters - M[a, b]) else 0
    return M

def kwiksort_aggregation(ranks):
    """
    KwikSort Kemeny approximation (Ailon-Charikar-Newman 2005).

    Input:
        ranks[v, c] = position of candidate c for voter v
    Output:
        list of candidates in consensus order (best → worst)
    """
    m = ranks.shape[1]
    M = compute_majority_matrix(ranks)
    candidates = list(range(m))
    return kwiksort_rec(candidates, M)

def kwiksort_rec(cands, M):
    """Recursive KwikSort on subset of candidates.
       cands: list of candidate indices
       M[a,b]: majority tournament matrix (numpy array or list of lists)
    """
    if len(cands) <= 1:
        return cands

    # pick pivot at random
    pivot = random.choice(cands)

    left = []
    right = []

    # partition based on majority tournament
    for c in cands:
        if c == pivot:
            continue
        if M[c, pivot] == 1:   # c beats pivot → goes left
            left.append(c)
        else:                  # pivot beats c → goes right
            right.append(c)

    # recurse and concatenate
    return kwiksort_rec(left, M) + [pivot] + kwiksort_rec(right, M)

