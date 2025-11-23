import time
import numpy as np
import matplotlib.pyplot as plt
from dataset_generator import generate_uniform, generate_cycle_heavy, generate_adversarial_bad_case
from soc_loader import load_soc_strict_orders

from algorithms import (
    kemeny_young_exact,
    borda,
    copeland,
    majority_sort,
    footrule_optimal,
    kendall_tau_distance,
    plackett_luce_mle,
    ranked_pairs,
    kemeny_local_search,
    schulze
)

# ------------ Configuration -------------
M_VOTERS = 2000             # fixed m
N_SIZES = [4, 5, 6, 7, 8, 9, 10, 15, 20, 25]  # vary number of candidates
PHI = 0.95               # Mallows noise
REPEATS_FAST = 3
REPEATS_SLOW = 1

# ------------ Kemeny score ---------------
def kemeny_score(order, ranks):
    inv = np.zeros(len(order), dtype=int)
    for pos, cand in enumerate(order):
        inv[cand] = pos
    return sum(kendall_tau_distance(inv, row) for row in ranks)


# ------------ Timer helper ---------------
def time_algorithm(func, ranks, repeats=1):
    start = time.perf_counter()
    for _ in range(repeats):
        func(ranks)
    end = time.perf_counter()
    return (end - start) / repeats


# ------------- Main benchmark ------------
def main():
    np.random.seed(42)

    # runtime storage
    rt_kem, rt_bor, rt_cop = [], [], []
    rt_foot, rt_ms = [], []

    # approximation ratio storage
    ar_bor, ar_cop, ar_foot, ar_ms = [], [], [], []

    rt_pl, rt_rp, rt_ls, rt_sch = [], [], [], []

    ar_pl, ar_rp, ar_ls, ar_sch = [], [], [], []


    for n in N_SIZES:
        print(f"\n=== n = {n} candidates, m = {M_VOTERS} voters ===")
        ranks = generate_adversarial_bad_case(M_VOTERS, n)
        # path = "/home/sacrifice/kemeny-evaluation/python code/00014_sushi/00014-00000001.soc"   # adjust path if needed
        # ranks = load_soc_strict_orders(path)

        # ------ Kemeny exact ------
        t_kem = time_algorithm(kemeny_young_exact, ranks, repeats=REPEATS_SLOW)
        kem_obj, kem_order, _ = kemeny_young_exact(ranks)
        opt_score = kemeny_score(kem_order, ranks)

        rt_kem.append(t_kem)
        print(f"Kemeny ILP    : {t_kem:.4f} s, score = {opt_score}")

        # ------ Borda ------
        t_b = time_algorithm(borda, ranks, repeats=REPEATS_FAST)
        bor_order = borda(ranks)
        bor_score = kemeny_score(bor_order, ranks)
        rt_bor.append(t_b)
        ar_bor.append(opt_score / bor_score)
        print(f"Borda         : {t_b:.4f} s, ratio = {opt_score/bor_score:.3f}")

        # ------ Copeland ------
        t_c = time_algorithm(copeland, ranks, repeats=REPEATS_FAST)
        cop_order = copeland(ranks)
        cop_score = kemeny_score(cop_order, ranks)
        rt_cop.append(t_c)
        ar_cop.append(opt_score / cop_score)
        print(f"Copeland      : {t_c:.4f} s, ratio = {opt_score/cop_score:.3f}")

        # ------ Footrule ------
        t_f = time_algorithm(footrule_optimal, ranks, repeats=REPEATS_FAST)
        foot_order = footrule_optimal(ranks)
        foot_score = kemeny_score(foot_order, ranks)
        rt_foot.append(t_f)
        ar_foot.append(opt_score / foot_score)
        print(f"Footrule      : {t_f:.4f} s, ratio = {opt_score/foot_score:.3f}")

        # ------ MajoritySort ------
        t_m = time_algorithm(majority_sort, ranks, repeats=REPEATS_FAST)
        ms_order = majority_sort(ranks)
        ms_score = kemeny_score(ms_order, ranks)
        rt_ms.append(t_m)
        ar_ms.append(opt_score / ms_score)
        print(f"majority_bubble_sort     : {t_m:.4f} s, ratio = {opt_score/ms_score:.3f}")

        # # ------ Plackett–Luce ------
        # t_pl = time_algorithm(plackett_luce_mle, ranks, repeats=REPEATS_FAST)
        # pl_order, pl_true_params = plackett_luce_mle(ranks)
        # pl_score = kemeny_score(pl_order, ranks)

        # rt_pl.append(t_pl)
        # ar_pl.append(opt_score / pl_score)
        # print(f"Plackett–Luce : {t_pl:.4f} s, ratio = {opt_score/pl_score:.3f}")

        # ------ Ranked Pairs ------
        t_rp = time_algorithm(ranked_pairs, ranks, repeats=REPEATS_FAST)
        rp_order = ranked_pairs(ranks)
        rp_score = kemeny_score(rp_order, ranks)

        rt_rp.append(t_rp)
        ar_rp.append(opt_score / rp_score)
        print(f"Ranked Pairs  : {t_rp:.4f} s, ratio = {opt_score/rp_score:.3f}")

        # ------ Local Search ------
        # t_ls = time_algorithm(kemeny_local_search, ranks, repeats=REPEATS_FAST)
        # score, ls_order = kemeny_local_search(ranks)
        # ls_score = kemeny_score(ls_order, ranks)

        # rt_ls.append(t_ls)
        # ar_ls.append(opt_score / ls_score)
        # print(f"Local Search  : {t_ls:.4f} s, ratio = {opt_score/ls_score:.3f}")

        # ------ Schulze ------
        t_sch = time_algorithm(schulze, ranks, repeats=REPEATS_FAST)
        sch_order = schulze(ranks)
        sch_score = kemeny_score(sch_order, ranks)

        rt_sch.append(t_sch)
        ar_sch.append(opt_score / sch_score)
        print(f"Schulze       : {t_sch:.4f} s, ratio = {opt_score/sch_score:.3f}")


    colors = {
        "kem": "black",
        "borda": "#1f77b4",      # blue
        "copeland": "#ff7f0e",   # orange
        "footrule": "#2ca02c",   # green
        "majority_sort": "#d62728",  # red
        "rp": "#9467bd",         # purple
        "schulze": "#8c564b",    # brown
        # "pl": "#e377c2",       # pink (if you re-enable it)
        # "ls": "#7f7f7f",       # gray  (if you re-enable it)
    }


    # ------------ Plot results ------------
    plt.figure(figsize=(12, 5))

    # ---------------- Runtime plot ----------------
    plt.subplot(1, 2, 1)
    plt.plot(N_SIZES, rt_kem,  marker="o", color=colors["kem"],      label="Kemeny (ILP)")
    plt.plot(N_SIZES, rt_bor,  marker="o", color=colors["borda"],    label="Borda")
    plt.plot(N_SIZES, rt_cop,  marker="o", color=colors["copeland"], label="Copeland")
    plt.plot(N_SIZES, rt_foot, marker="o", color=colors["footrule"], label="Footrule")
    plt.plot(N_SIZES, rt_ms,   marker="o", color=colors["majority_sort"],label="majority_sort")
    plt.plot(N_SIZES, rt_rp,   marker="o", color=colors["rp"],       label="Ranked Pairs")
    plt.plot(N_SIZES, rt_sch,  marker="o", color=colors["schulze"],  label="Schulze")

    plt.xlabel("Number of candidates (n)")
    # plt.yscale("log")
    plt.ylabel("Runtime (log seconds)")
    plt.title(f"Runtime vs n (m = {5000}, sushi dataset)")
    plt.grid(True)
    plt.legend()

    # ---------------- Approximation plot ----------------
    plt.subplot(1, 2, 2)
    plt.plot(N_SIZES, ar_bor,  marker="o", color=colors["borda"],        label="Borda")
    plt.plot(N_SIZES, ar_cop,  marker="o", color=colors["copeland"],     label="Copeland")
    plt.plot(N_SIZES, ar_foot, marker="o", color=colors["footrule"],     label="Footrule")
    plt.plot(N_SIZES, ar_ms,   marker="o", color=colors["MajoritySort"], label="MajoritySort")
    plt.plot(N_SIZES, ar_rp,   marker="o", color=colors["rp"],           label="Ranked Pairs")
    plt.plot(N_SIZES, ar_sch,  marker="o", color=colors["schulze"],      label="Schulze")

    plt.xlabel("Number of candidates (n)")
    plt.ylabel("Approximation ratio (Kemeny / method)")
    plt.ylim(0, 1.05)
    plt.title("Approximation Quality vs n")
    plt.grid(True)
    plt.legend()

    plt.tight_layout()
    plt.savefig("benchmark_n_only.png", dpi=300)
    print("Saved to benchmark_n_only.png")

        # =====================================================
    #   C. Kendall–Tau Distance Heatmap (Improved colours)
    # =====================================================

    algo_names = ["Kemeny", "Borda", "Copeland", "Footrule",
                  "MajoritySort", "Ranked Pairs", "Schulze"]
    algo_orders = [kem_order, bor_order, cop_order,
                   foot_order, ms_order, rp_order, sch_order]

    k = len(algo_orders)
    ktd_matrix = np.zeros((k, k), dtype=float)

    # compute distances
    for i in range(k):
        for j in range(k):
            ktd_matrix[i, j] = kendall_tau_distance(algo_orders[i], algo_orders[j])

    plt.figure(figsize=(7, 6))

    # ⭐ Better colormap
    im = plt.imshow(ktd_matrix, cmap="viridis")

    # colorbar
    cbar = plt.colorbar(im)
    cbar.set_label("Kendall–Tau Distance", rotation=270, labelpad=15)

    # axis labels
    plt.xticks(range(k), algo_names, rotation=45, ha="right")
    plt.yticks(range(k), algo_names)

    # ⭐ Better annotation: bold text + outline for readability
    import matplotlib.patheffects as pe
    for i in range(k):
        for j in range(k):
            plt.text(
                j, i, f"{ktd_matrix[i, j]:.0f}",
                ha="center", va="center",
                fontsize=10, color="white",
                path_effects=[pe.withStroke(linewidth=2, foreground="black")]
            )

    plt.title("Algorithm Similarity (Kendall–Tau Distance Matrix)")
    plt.tight_layout()
    plt.savefig("heatmap_kendall_tau.png", dpi=300)
    print("Saved heatmap to heatmap_kendall_tau.png")



if __name__ == "__main__":
    main()
