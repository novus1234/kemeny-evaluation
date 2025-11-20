# benchmark.py
import time
import numpy as np
import matplotlib.pyplot as plt

from soc_loader import load_soc_strict_orders
from algorithms import (
    kemeny_young_exact,
    borda,
    copeland,
    quicksort_approx,
    footrule_optimal,
    # plackett_luce_mle,   # uncomment if you have choix installed
    ranked_pairs,
    schulze,
)
from dataset_generator import generate_mallows

def time_algorithm(func, ranks, repeats=1, **kwargs):
    """
    Measure runtime of func(ranks, **kwargs) with simple repetition.
    Returns average time in seconds.
    """
    start = time.perf_counter()
    for _ in range(repeats):
        func(ranks, **kwargs)
    end = time.perf_counter()
    return (end - start) / repeats


def main():
    path = "/home/sacrifice/kemeny-evaluation/python code/00004_netflix/00004-00000004.soc"   # adjust path if needed
    rankings = generate_mallows(n_voters=5000, n_cands=100, phi=0.7)
    n_voters, n_cands = rankings.shape
    print(f"Loaded {n_voters} voters, {n_cands} candidates")

    # we vary number of candidates k (3..n_cands)
    candidate_sizes = list(range(3, n_cands + 1))

    times_kemeny = []
    times_borda = []
    times_copeland = []
    times_quicksort = []
    times_footrule = []
    # times_pl = []

    times_ranked_pairs = []   # NEW
    times_schulze = []        # NEW

    for k in candidate_sizes:
        ranks_k = rankings[:, :k]  # restrict to first k candidates

        print(f"\n--- k = {k} candidates ---")
        # You can reduce repeats for expensive methods
        # WARNING: Kemeny ILP may blow up beyond ~10 candidates
        t_borda = time_algorithm(borda, ranks_k, repeats=3)
        t_copeland = time_algorithm(copeland, ranks_k, repeats=3)
        t_foot = time_algorithm(footrule_optimal, ranks_k, repeats=3)
        t_qs = time_algorithm(quicksort_approx, ranks_k, repeats=3)

        times_borda.append(t_borda)
        times_copeland.append(t_copeland)
        times_footrule.append(t_foot)
        times_quicksort.append(t_qs)

        print(f"Borda       : {t_borda:.6f} s")
        print(f"Copeland    : {t_copeland:.6f} s")
        print(f"Footrule    : {t_foot:.6f} s")
        print(f"Quicksort   : {t_qs:.6f} s")

        # Kemeny ILP: only try up to some small k (e.g., 3–10)
        if k <= 9:
            t_kem = time_algorithm(kemeny_young_exact, ranks_k, repeats=1)
            times_kemeny.append(t_kem)
            print(f"Kemeny ILP  : {t_kem:.6f} s")
        else:
            times_kemeny.append(np.nan)

        # ------ Ranked Pairs ------
        t_rp = time_algorithm(ranked_pairs, ranks_k, repeats=3)
        times_ranked_pairs.append(t_rp)
        print(f"Ranked Pairs: {t_rp:.6f} s")

        # ------ Schulze Method ------
        t_sch = time_algorithm(schulze, ranks_k, repeats=3)
        times_schulze.append(t_sch)
        print(f"Schulze     : {t_sch:.6f} s")

        # True Plackett–Luce MLE (optional)
        # try:
        #     t_pl = time_algorithm(plackett_luce_mle, ranks_k, repeats=1)
        #     times_pl.append(t_pl)
        #     print(f"PL (MLE)    : {t_pl:.6f} s")
        # except Exception as e:
        #     times_pl.append(np.nan)
        #     print(f"PL (MLE)    : error: {e}")

    # ---- Plot runtime vs candidates ----
    plt.figure(figsize=(10, 6))
    plt.plot(candidate_sizes, times_borda, label="Borda")
    plt.plot(candidate_sizes, times_copeland, label="Copeland")
    plt.plot(candidate_sizes, times_footrule, label="Footrule")
    plt.plot(candidate_sizes, times_quicksort, label="Quicksort heuristic")
    plt.plot(candidate_sizes, times_kemeny, label="Kemeny ILP (exact)")
    plt.plot(candidate_sizes, times_ranked_pairs, label="Ranked Pairs")
    plt.plot(candidate_sizes, times_schulze, label="Schulze")

    # if times_pl:
    #     plt.plot(candidate_sizes, times_pl, label="Plackett–Luce MLE")

    plt.xlabel("Number of candidates (k)")
    plt.yscale("log")
    plt.ylabel("Runtime (seconds, log scale)")
    plt.title("Runtime vs number of candidates on Sushi dataset")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('my_plot.png')


if __name__ == "__main__":
    main()
