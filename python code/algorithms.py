# algorithms.py

import numpy as np
from itertools import combinations, permutations, chain
from pulp import (
    LpProblem, LpVariable, LpBinary, lpSum, LpMinimize,
    value, LpStatusOptimal
)

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
    n_cand = ranks.shape[1]
    best_perm = None
    best_score = 10**18

    for perm in permutations(range(n_cand)):
        inv = np.zeros(n_cand, dtype=int)
        for pos, cand in enumerate(perm):
            inv[cand] = pos

        score = sum(kendall_tau_distance(inv, voter) for voter in ranks)

        if score < best_score:
            best_score = score
            best_perm = perm

    return best_perm, best_score

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
    return np.argsort(-params), params

# ----------------------------------------------------------
# 6. Quicksort Heuristic (Dwork et al. 2001)
# ----------------------------------------------------------
def quicksort_approx(ranks):
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

def kemeny_local_search(ranks: np.ndarray,
                        max_restarts: int = 20,
                        max_iters_no_improve: int = 1000) -> tuple[int, np.ndarray]:
    """
    Repeated 2-opt local search for Kemeny-Young (excellent heuristic)
    Often finds the exact optimum in practice.
    """
    n_voters, n_cand = ranks.shape

    def kemeny_score(order: np.ndarray) -> int:
        """Fast Kemeny score for a given order (permutation of candidates)"""
        score = 0
        for i in range(n_cand):
            for j in range(i+1, n_cand):
                a, b = order[i], order[j]
                # Number of voters who disagree with a > b
                score += np.sum(ranks[:,a] > ranks[:,b])
        return score

    best_score = float('inf')
    best_order = None

    for restart in range(max_restarts):
        # Start from a good initial ranking (Borda is excellent)
        initial_scores = np.sum(ranks, axis=0)
        current_order = np.argsort(initial_scores)
        current_score = kemeny_score(current_order)

        iters_no_improve = 0
        while iters_no_improve < max_iters_no_improve:
            improved = False
            for i in range(n_cand - 1):
                # Try swapping adjacent pair
                current_order[i], current_order[i+1] = current_order[i+1], current_order[i]
                new_score = kemeny_score(current_order)

                if new_score < current_score:
                    current_score = new_score
                    improved = True
                    iters_no_improve = 0
                else:
                    # Revert swap
                    current_order[i], current_order[i+1] = current_order[i+1], current_order[i]
                    iters_no_improve += 1

            if not improved:
                break

        if current_score < best_score:
            best_score = current_score
            best_order = current_order.copy()

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

def kemeny_parameterized_exact(ranks: np.ndarray) -> tuple[int, list]:
    """
    Exact Kemeny using DP over subsets – O(2^m * m^2 * n)
    Theorem 4.6 (Betzler et al., 2009)

    Input: ranks – (n_voters, m_candidates), lower = better
    Output: (optimal_score, optimal_order_as_list best→worst)
    """
    n, m = ranks.shape

    # Precompute cost of placing a before b in final ranking
    # cost[a,b] = number of voters who prefer b > a (penalty if a before b)
    cost = np.zeros((m, m), dtype=int)
    for a in range(m):
        for b in range(m):
            if a != b:
                cost[a,b] = np.sum(ranks[:,a] > ranks[:,b])

    # All nonempty subsets
    subsets = list(chain.from_iterable(combinations(range(m), k) for k in range(1, m+1)))
    subset_id = {frozenset(s): i for i, s in enumerate(subsets)}
    N = len(subsets)

    # dp[S] = min Kemeny score for subset S
    dp = [float('inf')] * N
    # prev[S] = last candidate placed to achieve dp[S]
    prev = [-1] * N

    # Base case: singletons
    for i in range(m):
        S = frozenset([i])
        dp[subset_id[S]] = 0
        prev[subset_id[S]] = -1  # no previous

    # Fill DP
    for size in range(2, m+1):
        for S in combinations(range(m), size):
            S = frozenset(S)
            sid = subset_id[S]
            for last in S:                                      # try every possible last candidate
                remaining = S - {last}
                if not remaining:
                    continue
                rid = subset_id[remaining]
                added_cost = sum(cost[last, x] for x in remaining)   # penalty: last before all previous
                total = dp[rid] + added_cost
                if total < dp[sid]:
                    dp[sid] = total
                    prev[sid] = last

    # Reconstruct solution
    full = frozenset(range(m))
    best_score = dp[subset_id[full]]

    order = []
    current = full
    while current:
        last = prev[subset_id[current]]
        order.append(last)                  # last can be -1 only for singletons → ignore later
        current = current - {last}

    order = [x for x in reversed(order) if x != -1]   # reverse + drop the dummy -1
    return int(best_score), order
