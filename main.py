import numpy as np
from algorithms import (
    kemeny_young_exact,
    borda,
    copeland,
    majority_sort,
    footrule_optimal,
    plackett_luce_mle,
    ranked_pairs,
    kemeny_local_search,
    schulze,
    kemeny_dp_by_candidates,
    kwiksort_aggregation,
    kemeny_bruteforce,
    random_ranking
)

# ------------------------------------------------
# Helpers to normalize and pretty-print rankings
# ------------------------------------------------

def normalize_indices(result):
    cleaned = []
    for item in result:
        if isinstance(item, tuple):
            idx = int(item[1])
        elif isinstance(item, str):
            idx = int(item)
        elif hasattr(item, "dtype"):
            idx = int(item)
        else:
            idx = int(item)
        cleaned.append(idx - 1)
    return cleaned


def pretty_print_table(title, methods, names):
    print("\n" + "=" * 70)
    print(title)
    print("=" * 70)
    for method_name, ranking in methods:
        pretty = " > ".join(names[i] for i in ranking)
        print(f"{method_name:22s} : {pretty}")


# ------------------------------------------------
# Main
# ------------------------------------------------

if __name__ == "__main__":

    players = ["Alicia", "Ginny", "Gwendolyn", "Robin", "Debbie"]

    rankings = np.array([
        [0,1,2,3,4],
        [0,1,3,2,4],
        [4,1,2,0,3],
        [4,1,0,2,3],
        [4,1,3,2,0],
    ])
    rankings_fixed = rankings + 1  # ILP requires 1-based

    # ---------- EXACT KEMENY ----------
    objective, consensus, kendall_sum = kemeny_young_exact(rankings_fixed)
    kemeny_dp_by_candidates_res = kemeny_dp_by_candidates(rankings)
    bruteforce_res = kemeny_bruteforce(rankings)

    # ---------- CLASSICAL METHODS ----------
    borda_res = borda(rankings)
    copeland_res = copeland(rankings)
    footrule_res = footrule_optimal(rankings)
    majority_sort_res = majority_sort(rankings)
    ranked_pairs_res = ranked_pairs(rankings)
    kemeny_local_search_res  = kemeny_local_search(rankings)
    schulze_res = schulze(rankings)
    kwiksort_res = kwiksort_aggregation(rankings)
    pick_a_perm_res = random_ranking(rankings)
    try:
        pl_true_order, pl_true_params = plackett_luce_mle(rankings)
        pl_true_order = list(pl_true_order)
    except Exception as e:
        pl_true_order = None

    # Collect all methods
    all_methods = [
        ("Kemeny (ILP exact)",     list(consensus)),
        ("Parameterised exact",     list(kemeny_dp_by_candidates_res[1])),
        ("Bruteforce",     list(bruteforce_res)),
        ("Borda (classical)",      list(borda_res)),
        ("Kwiksort",      list(kwiksort_res)),
        ("Copeland (classical)",   list(copeland_res)),
        ("Footrule (optimal)",     list(footrule_res)),
        ("MajoritySort (heuristic)",  list(majority_sort_res)),
        ("Ranked pairs (heuristic)",  list(ranked_pairs_res)),
        ("Local Search (heuristic)",  list(kemeny_local_search_res)),
        ("schulze",  list(schulze_res)),
        ("Pick-a-Perm",  list(pick_a_perm_res)),
    ]

    if pl_true_order is not None:
        all_methods.append(("Plackett-Luce (MLE)",  pl_true_order))

    # Pretty print everything
    pretty_print_table("ALL RANKING METHODS", all_methods, players)

    print("\nExact Kemeny Objective (pairwise disagreements):", objective)
    print("Exact Kendall-tau sum:", kendall_sum)

    # TODO add bruteforce, rankedpairs, local search, parametrised algorithms
