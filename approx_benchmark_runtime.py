# approx_benchmark_runtime.py
import time
import numpy as np
import matplotlib.pyplot as plt

# ---------- Import approximate algorithms ----------
from algorithms import (
    borda,
    copeland,
    footrule_optimal,
    majority_sort,
    ranked_pairs,
    kemeny_local_search,
    schulze,
    kwiksort_aggregation,
    random_ranking,
    plackett_luce_mle,    # optional; comment out if choix not installed
)

# ---------- Dataset generators ----------
from dataset_generator import (
    generate_adversarial_bad_case,
    generate_cycle_heavy,
    generate_block_model,
    generate_plackett_luce,
    generate_bradley_terry,
    generate_mallows,
    generate_uniform,
)

# -------------------------------------------
def time_algorithm(func, ranks, repeats=1, **kwargs):
    """
    Measure average runtime of a function.
    """
    start = time.perf_counter()
    for _ in range(repeats):
        func(ranks, **kwargs)
    end = time.perf_counter()
    return (end - start) / repeats


# -------------------------------------------
def choose_dataset(name, n_voters, n_cands):
    """
    Map dataset name → generator function.
    """
    if name == "adversarial":
        return generate_adversarial_bad_case(n_voters, n_cands)
    elif name == "cycle":
        return generate_cycle_heavy(n_voters, n_cands)
    elif name == "block":
        return generate_block_model(n_voters, n_cands)
    elif name == "pl":
        return generate_plackett_luce(n_voters, n_cands)
    elif name == "bt":
        return generate_bradley_terry(n_voters, n_cands)
    elif name == "mallows":
        return generate_mallows(n_voters, n_cands, phi=0.7)
    elif name == "uniform":
        return generate_uniform(n_voters, n_cands)
    else:
        raise ValueError(f"Unknown dataset type: {name}")


# -------------------------------------------
def main():

    # -------- SETTINGS --------
    DATASET = "uniform"      # choose dataset type
    N_VOTERS = 2000          # total voters
    MAX_CANDS = 150          # approximate methods scale well

    print(f"Dataset = {DATASET}, Voters = {N_VOTERS}")

    candidate_sizes = list(range(3, MAX_CANDS + 1, 5))

    # Record runtimes
    times_borda = []
    times_copeland = []
    times_footrule = []
    times_majority = []
    times_ranked_pairs = []
    times_local_search = []
    times_schulze = []
    times_kwiksort = []
    times_pl = []   # optional
    times_random = []

    # -------- BENCHMARK LOOP --------
    for k in candidate_sizes:
        print(f"\n--- Candidates: k = {k} ---")

        ranks = choose_dataset(DATASET, N_VOTERS, k)

        # ------ Classical methods ------
        tb = time_algorithm(borda, ranks, repeats=3)
        tc = time_algorithm(copeland, ranks, repeats=3)
        tf = time_algorithm(footrule_optimal, ranks, repeats=3)
        tms = time_algorithm(majority_sort, ranks, repeats=3)

        trp = time_algorithm(ranked_pairs, ranks, repeats=1)
        tls = time_algorithm(kemeny_local_search, ranks, repeats=1)
        tsch = time_algorithm(schulze, ranks, repeats=1)
        tkw = time_algorithm(kwiksort_aggregation, ranks, repeats=3)
        trnd = time_algorithm(random_ranking, ranks, repeats=3)

        times_borda.append(tb)
        times_copeland.append(tc)
        times_footrule.append(tf)
        times_majority.append(tms)
        times_ranked_pairs.append(trp)
        times_local_search.append(tls)
        times_schulze.append(tsch)
        times_kwiksort.append(tkw)
        times_random.append(trnd)

        print(f"Borda         : {tb:.6f} s")
        print(f"Copeland      : {tc:.6f} s")
        print(f"Footrule      : {tf:.6f} s")
        print(f"MajoritySort  : {tms:.6f} s")
        print(f"RankedPairs   : {trp:.6f} s")
        print(f"LocalSearch   : {tls:.6f} s")
        print(f"Schulze       : {tsch:.6f} s")
        print(f"KwikSort      : {tkw:.6f} s")
        print(f"RandomRank   : {trnd:.6f} s")


        # Optional PL
        if k <= 70:
            try:
                tpl = time_algorithm(plackett_luce_mle, ranks, repeats=1)
                times_pl.append(tpl)
                print(f"Plackett-Luce : {tpl:.6f} s")
            except Exception:
                times_pl.append(np.nan)
                print("Plackett-Luce : FAILED")
        else:
            times_pl.append(np.nan)
            print("Plackett-Luce : SKIPPED (k > 70)")

    # -------- PLOT RUNTIMES --------
    plt.figure(figsize=(10, 6))

    plt.plot(candidate_sizes, times_borda, label="Borda")
    plt.plot(candidate_sizes, times_copeland, label="Copeland")
    plt.plot(candidate_sizes, times_footrule, label="Footrule (Optimal)")
    plt.plot(candidate_sizes, times_majority, label="MajoritySort (AMS)")
    plt.plot(candidate_sizes, times_ranked_pairs, label="Ranked Pairs")
    plt.plot(candidate_sizes, times_local_search, label="Local Search")
    plt.plot(candidate_sizes, times_schulze, label="Schulze")
    plt.plot(candidate_sizes, times_kwiksort, label="KwikSort")
    plt.plot(candidate_sizes, times_random, label="Pick-a-Perm")

    if times_pl:
        plt.plot(candidate_sizes, times_pl, label="Plackett-Luce MLE")

    plt.xlabel("Number of Candidates (k)")
    plt.ylabel("Runtime (seconds, log scale)")
    plt.yscale("log")
    plt.title(f"Approximate Algorithms Runtime on {DATASET} Dataset")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig("approx_runtime.png")

    print("\nPlot saved as approx_runtime.png")


# -------------------------------------------
if __name__ == "__main__":
    main()
