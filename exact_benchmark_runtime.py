# exact_benchmark.py
import time
import numpy as np
import matplotlib.pyplot as plt

# import your algorithms
from algorithms import (
    kemeny_young_exact,
    kemeny_bruteforce,
    kemeny_dp_by_candidates,
)

# import dataset generators
from dataset_generator import (
    generate_adversarial_bad_case,
    generate_cycle_heavy,
    generate_block_model,
    generate_plackett_luce,
    generate_bradley_terry,
    generate_mallows,
    generate_uniform,
)

# ----------------------------------------------------------
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


# ----------------------------------------------------------
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


# ----------------------------------------------------------
def main():
    # ---- SETTINGS ----
    DATASET = "uniform"
    N_VOTERS = 1000       # normally 200–1000 is enough for exact benchmarking
    MAX_CANDS = 25       # exact algorithms blow up if too large

    print(f"Dataset = {DATASET}, Voters = {N_VOTERS}")

    candidate_sizes = list(range(3, MAX_CANDS + 1))

    times_ilp = []           # Kemeny (ILP or pairwise DP)
    times_bruteforce = []    # m! brute-force
    times_dp = []            # DP over subsets O(2^m)

    # ---- Run benchmark ----
    for k in candidate_sizes:
        print(f"\n--- Candidates: k = {k} ---")

        # generate dataset
        ranks = choose_dataset(DATASET, N_VOTERS, k)

        # --- ILP (exact) ---
        t_ilp = time_algorithm(kemeny_young_exact, ranks, repeats=1)
        times_ilp.append(t_ilp)
        print(f"Kemeny ILP            : {t_ilp:.6f} s")

        # --- Brute force (m! permutations) ---
        if k <= 9:
            t_bf = time_algorithm(kemeny_bruteforce, ranks, repeats=1)
            times_bruteforce.append(t_bf)
            print(f"Kemeny Brute Force    : {t_bf:.6f} s")
        else:
            times_bruteforce.append(np.nan)
            print("Kemeny Brute Force    : skipped")

        # --- DP-by-candidates (2^m) ---
        if k <= 18:
            t_dp = time_algorithm(kemeny_dp_by_candidates, ranks, repeats=1)
            times_dp.append(t_dp)
            print(f"Kemeny DP (2^m)       : {t_dp:.6f} s")
        else:
            times_dp.append(np.nan)
            print("Kemeny DP (2^m)       : skipped")

    # ---- Plot runtime ----
    plt.figure(figsize=(10, 6))

    plt.plot(candidate_sizes, times_ilp, label="Kemeny ILP (pairwise exact)")
    plt.plot(candidate_sizes, times_bruteforce, label="Kemeny Brute Force (m!)")
    plt.plot(candidate_sizes, times_dp, label="Kemeny DP (2ᵐ)")

    plt.xlabel("Number of Candidates (k)")
    plt.ylabel("Runtime (seconds, log scale)")
    plt.yscale("log")
    plt.title(f"Exact Kemeny Runtime on {DATASET} Dataset")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig("exact_kemeny_runtime.png")

    print("\nPlot saved as exact_kemeny_runtime.png")


# ----------------------------------------------------------
if __name__ == "__main__":
    main()
