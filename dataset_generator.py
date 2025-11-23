import numpy as np


# -------------------------------------------------------
# 1. Uniform random rankings
# -------------------------------------------------------
def generate_uniform(n_voters, n_cands):
    ranks = np.zeros((n_voters, n_cands), dtype=int)
    for v in range(n_voters):
        perm = np.random.permutation(n_cands)
        ranks[v] = np.argsort(perm)   # convert to rank positions
    return ranks


# -------------------------------------------------------
# 2. Mallows model (Kendall distance)
# -------------------------------------------------------
def generate_mallows(n_voters, n_cands, phi):
    """
    Mallows model around central ranking [0,1,2,...].
    phi = 0 → identical rankings
    phi = 1 → completely random
    """
    central = np.arange(n_cands)
    rankings = []

    for _ in range(n_voters):
        sigma = []
        remaining = list(central)
        for i in range(n_cands, 0, -1):
            probs = np.array([phi ** j for j in range(i)])
            probs /= probs.sum()
            k = np.random.choice(i, p=probs)   # position from front
            sigma.append(remaining.pop(k))
        sigma = np.array(sigma)
        rankings.append(np.argsort(sigma))

    return np.array(rankings)


# -------------------------------------------------------
# 3. Bradley–Terry pairwise model
# -------------------------------------------------------
def generate_bradley_terry(n_voters, n_cands, strengths=None):
    """
    strengths: an array of length n_cands, larger means more preferred.
    If None, sample random strengths.
    """
    if strengths is None:
        strengths = np.random.uniform(0.5, 2.0, n_cands)

    rankings = []

    # Generate rankings via pairwise tournament sorting
    for _ in range(n_voters):
        # Simulate stochastic wins
        order = list(range(n_cands))
        def win_prob(i, j):
            return strengths[i] / (strengths[i] + strengths[j])

        # sort by sampling pairwise outcomes
        for i in range(n_cands):
            for j in range(i + 1, n_cands):
                if np.random.rand() < win_prob(order[j], order[i]):
                    order[i], order[j] = order[j], order[i]

        rankings.append(np.argsort(order))

    return np.array(rankings)


# -------------------------------------------------------
# 4. Plackett–Luce model
# -------------------------------------------------------
def generate_plackett_luce(n_voters, n_cands, weights=None):
    """
    weights: PL parameters. Higher weight → more preferred.
    If None, sample random weights > 0.
    """
    if weights is None:
        weights = np.random.uniform(0.1, 2.0, n_cands)

    rankings = []
    for _ in range(n_voters):
        remaining = list(range(n_cands))
        ranking = []

        w = weights.copy()

        while remaining:
            probs = w[remaining] / np.sum(w[remaining])
            winner = np.random.choice(remaining, p=probs)
            ranking.append(winner)
            remaining.remove(winner)

        rankings.append(np.argsort(ranking))

    return np.array(rankings)


# -------------------------------------------------------
# 5. Block model (clusters of similarity)
# -------------------------------------------------------
def generate_block_model(n_voters, n_cands, n_blocks=3, noise=0.1):
    """
    Candidates grouped into blocks.
    Voters prefer earlier blocks with high probability.
    Within a block, random.
    """
    block_size = n_cands // n_blocks
    blocks = [list(range(i*block_size, (i+1)*block_size)) for i in range(n_blocks)]

    rankings = []

    for _ in range(n_voters):
        # Choose block order with noise
        block_order = list(range(n_blocks))
        if np.random.rand() < noise:
            np.random.shuffle(block_order)

        ranking = []
        for b in block_order:
            within = blocks[b].copy()
            np.random.shuffle(within)
            ranking.extend(within)

        rankings.append(np.argsort(ranking))

    return np.array(rankings)

def generate_cycle_heavy(n_voters, n_cands, noise=0.2):
    """
    Generate a profile with MANY Condorcet cycles.

    Strongly non-transitive preferences:
    Voter groups prefer different cyclic orders:
      Group 1: 0 > 1 > 2 > ... > n-1
      Group 2: 1 > 2 > 3 > ... > 0
      Group 3: 2 > 3 > 4 > ... > 1
      ...
    Add random perturbations with 'noise'.

    Output: rankings[voter][candidate] = position
    """
    rankings = []
    base_orders = []

    # create n cyclic base orders
    base = list(range(n_cands))
    for shift in range(n_cands):
        base_orders.append(base[shift:] + base[:shift])

    # assign voters into cycle groups
    group_sizes = np.random.multinomial(n_voters, [1/n_cands] * n_cands)

    for g in range(n_cands):
        order = base_orders[g]

        for _ in range(group_sizes[g]):
            # Add random swaps (noise)
            perm = order.copy()
            for i in range(n_cands):
                if np.random.rand() < noise:
                    j = np.random.randint(0, n_cands)
                    perm[i], perm[j] = perm[j], perm[i]

            # Convert to rank positions
            inv = np.zeros(n_cands, dtype=int)
            for pos, candidate in enumerate(perm):
                inv[candidate] = pos

            rankings.append(inv)

    return np.array(rankings)

def generate_adversarial_bad_case(n_voters, n_cands, noise=0.0):
    """
    Generate a classic Moulin / Conitzer-Sandholm adversarial profile.

    Construction:
        Candidates are arranged in a cycle with heavy voter blocks
        each enforcing a different strict ordering.

        Example for n_cands = 4:
            Block 1: 0 > 1 > 2 > 3
            Block 2: 1 > 2 > 3 > 0
            Block 3: 2 > 3 > 0 > 1
            Block 4: 3 > 0 > 1 > 2

        All blocks have equal size (or as equal as possible).
        This produces a *perfect Condorcet cycle*:
            0 ≻ 1, 1 ≻ 2, 2 ≻ 3, 3 ≻ 0.

    Effects:
        - No candidate is a Condorcet winner.
        - Pairwise majority matrix is exactly cyclic.
        - Borda, Copeland, Footrule, Quicksort all fail badly.
        - Kemeny ILP still finds the minimum-Kendall ranking.

    noise ∈ [0,1]: probability of random swap inside each ranking.
    """

    if n_cands < 3:
        raise ValueError("Need at least 3 candidates for cycles.")

    # Generate base cyclic orders
    base = list(range(n_cands))
    cyclic_orders = []
    for shift in range(n_cands):
        cyclic_orders.append(base[shift:] + base[:shift])

    # Divide voters into n_cands blocks
    group_sizes = np.random.multinomial(n_voters, [1/n_cands] * n_cands)

    rankings = []
    for g in range(n_cands):
        order = cyclic_orders[g]

        for _ in range(group_sizes[g]):
            # Optional noise
            perm = order.copy()
            if noise > 0:
                for i in range(n_cands):
                    if np.random.rand() < noise:
                        j = np.random.randint(0, n_cands)
                        perm[i], perm[j] = perm[j], perm[i]

            # Convert to rank positions
            inv = np.zeros(n_cands, dtype=int)
            for pos, cand in enumerate(perm):
                inv[cand] = pos

            rankings.append(inv)

    return np.array(rankings)
