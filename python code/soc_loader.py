# soc_loader.py
import numpy as np

def load_soc_strict_orders(path, expand=True):
    """
    Load a PrefLib .soc file (Strict Orders - Complete Lists)
    into a NumPy matrix of shape (n_voters, n_candidates),
    where entry [v, c] = rank position of candidate c (0 = best).

    If expand=False, returns (rankings, counts) where rankings[i]
    is one unique ranking, counts[i] its multiplicity.
    """
    pref_lines = []
    max_alt = 0

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            # preference lines look like: "13: 1,4,3,2"
            # so split on colon and check left part is a number
            if ":" in line and line.split(":", 1)[0].strip().isdigit():
                pref_lines.append(line)

    orders = []
    freqs = []

    for line in pref_lines:
        left, right = line.split(":", 1)
        count = int(left.strip())
        # remove braces just in case, though .soc shouldn't have ties
        right = right.replace("{", "").replace("}", "")
        alts = [int(x.strip()) for x in right.split(",") if x.strip()]
        max_alt = max(max_alt, max(alts))
        orders.append(alts)
        freqs.append(count)

    n_cands = max_alt
    rankings_unique = []

    # convert each ordering [a1,a2,...,a_n] into position vector
    # pos[candidate_id-1] = rank position (0 = top)
    for alts in orders:
        pos = np.zeros(n_cands, dtype=int)
        for rank_pos, alt_id in enumerate(alts):
            pos[alt_id - 1] = rank_pos
        rankings_unique.append(pos)

    rankings_unique = np.vstack(rankings_unique)
    freqs = np.array(freqs, dtype=int)

    if not expand:
        return rankings_unique, freqs

    # expand into one row per voter
    rows = []
    for r, count in zip(rankings_unique, freqs):
        for _ in range(count):
            rows.append(r.copy())

    rankings_full = np.vstack(rows)
    return rankings_full
