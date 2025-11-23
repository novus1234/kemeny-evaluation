import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import kendalltau

# ----- Import algorithms -----
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
    kwiksort_aggregation,
    random_ranking
)

# ----- Import dataset generators -----
from dataset_generator import (
    generate_uniform,
    generate_cycle_heavy,
    generate_adversarial_bad_case,
    generate_mallows
)

from soc_loader import load_soc_strict_orders

# ===========================================
#   Kendall–Tau correlation coefficient
# ===========================================
def kendall_tau_coeff(order1, order2):
    """Return Kendall–Tau correlation between two rankings."""
    m = len(order1)
    pos1 = np.zeros(m, dtype=int)
    pos2 = np.zeros(m, dtype=int)

    for i, c in enumerate(order1):
        pos1[c] = i
    for i, c in enumerate(order2):
        pos2[c] = i

    tau, _ = kendalltau(pos1, pos2)
    return tau


# ===========================================
# Select dataset generator
# ===========================================
def choose_dataset(name, n_voters, n_cands):
    path = "/home/sacrifice/kemeny-evaluation/real datasets/00014_sushi/00014-00000001.soc"   # adjust path if needed
    return load_soc_strict_orders(path)
    if name == "uniform":
        return generate_uniform(n_voters, n_cands)
    elif name == "adversarial":
        return generate_adversarial_bad_case(n_voters, n_cands, noise=0.2)
    elif name == "cycle":
        return generate_cycle_heavy(n_voters, n_cands)
    elif name == "mallows":
        return generate_mallows(n_voters, n_cands, phi=0.8)
    else:
        raise ValueError("Unknown dataset type")


# ===========================================
# MAIN BOXPLOT SCRIPT
# ===========================================
def main():

    DATASET = "mallows"        # <-- choose: uniform / adversarial / cycle / mallows
    N_VOTERS = 2000
    N_CANDS = 15               # <-- number of candidates per trial
    TRIALS = 10                # <-- more trials = smoother boxplots

    print(f"Dataset = {DATASET}, voters = {N_VOTERS}, candidates = {N_CANDS}")
    print(f"Running {TRIALS} random trials...\n")

    # Algorithms to test
    algorithms = {
        "Borda": borda,
        "Copeland": copeland,
        "Footrule": footrule_optimal,
        "MajoritySort": majority_sort,
        "RankedPairs": ranked_pairs,
        "Schulze": schulze,
        "KwikSort": kwiksort_aggregation,
        "LocalSearch": kemeny_local_search,
        "Random": random_ranking
    }

    # skip PL for large n
    if N_CANDS <= 70:
        algorithms["PlackettLuce"] = plackett_luce_mle

    # store results
    results = {name: [] for name in algorithms.keys()}

    # =======================
    #   Run experiments
    # =======================
    for t in range(TRIALS):
        print(f"Trial {t+1}/{TRIALS}")

        ranks = choose_dataset(DATASET, N_VOTERS, N_CANDS)

        # Exact Kemeny
        _, kem_order, _ = kemeny_young_exact(ranks)

        # Evaluate each method
        for name, func in algorithms.items():
            try:
                out = func(ranks)

                # some algorithms return tuple
                if isinstance(out, tuple):
                    order = out[0]    # (order, params)
                else:
                    order = out

                tau = kendall_tau_coeff(order, kem_order)
                results[name].append(tau)
            except Exception:
                results[name].append(np.nan)
                print(f"{name} failed.")

    # =======================
    #   Make Boxplot
    # =======================
    labels = list(results.keys())
    data = [results[name] for name in labels]

    plt.figure(figsize=(12, 6))
    plt.boxplot(data, labels=labels, showfliers=False)
    plt.ylabel("Kendall-Tau Correlation (τ)")
    plt.title(f"Approximation Quality Using Kendall-Tau (Dataset: {DATASET})")
    plt.grid(True, axis='y')
    plt.ylim(-1, 1)

    plt.tight_layout()
    plt.savefig("approx_quality_kendall_tau_boxplot.png", dpi=300)
    print("\nSaved plot as approx_quality_kendall_tau_boxplot.png")


if __name__ == "__main__":
    main()
