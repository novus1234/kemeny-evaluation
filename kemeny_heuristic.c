#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ranksfile.h"

#define MPERM 7   // Size of the local optimization window for small permutations

// =====================================================
// Utility: Swap two integers (used for permutation manipulation)
// =====================================================
static inline void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

// =====================================================
// Compute the total Kemeny score for a given ranking (perm)
// -----------------------------------------------------
// The score measures how consistent the ranking is with
// the pairwise preferences in rf->prefmat.
// Higher score = better agreement with voters.
// =====================================================
static double compute_score(RanksFile *rf, int *perm) {
    double score = 0.0;
    int n = rf->ncands;
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            int a = perm[i];
            int b = perm[j];
            score += (rf->prefmat[a][b] - rf->prefmat[b][a]);
        }
    }
    return score;
}

// =====================================================
// Compute partial Kemeny score for a subrange of candidates
// -----------------------------------------------------
// Used by the local permutation optimization to evaluate
// only a small "window" (lo to hi) instead of all candidates.
// =====================================================
static double compute_partial_score(RanksFile *rf, int *perm, int lo, int hi) {
    double score = 0.0;
    for (int i = lo; i < hi; i++) {
        for (int j = i + 1; j <= hi; j++) {
            int a = perm[i];
            int b = perm[j];
            score += (rf->prefmat[a][b] - rf->prefmat[b][a]);
        }
    }
    return score;
}

// =====================================================
// Move-Insert Heuristic
// -----------------------------------------------------
// For each candidate, tries moving it to a different position
// if it improves the score. Similar to a "hill-climbing" step.
//
// For each candidate i:
//   - Try inserting it at every other position j
//   - Calculate how much the Kemeny score changes (delta)
//   - Perform the best move if it increases the score
// =====================================================
static void move_insert(RanksFile *rf, int *perm, double *lastscore) {
    int n = rf->ncands;

    for (int i = 0; i < n; i++) {
        double best_delta = 0.0;
        int best_pos = i;

        // Try inserting perm[i] at every other position j
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            double delta = 0.0;

            // Calculate delta for moving candidate i to j
            if (j < i) {
                for (int k = j; k < i; k++)
                    delta += 2.0 * (rf->prefmat[perm[i]][perm[k]] - rf->prefmat[perm[k]][perm[i]]);
            } else {
                for (int k = i + 1; k <= j; k++)
                    delta += 2.0 * (rf->prefmat[perm[k]][perm[i]] - rf->prefmat[perm[i]][perm[k]]);
            }

            if (delta > best_delta) {
                best_delta = delta;
                best_pos = j;
            }
        }

        // If moving improves the score, perform the best move
        if (best_delta > 0.0) {
            int temp = perm[i];

            // Shift elements to make space for insertion
            if (best_pos < i) {
                memmove(&perm[best_pos + 1], &perm[best_pos], (i - best_pos) * sizeof(int));
                perm[best_pos] = temp;
            } else if (best_pos > i) {
                memmove(&perm[i], &perm[i + 1], (best_pos - i) * sizeof(int));
                perm[best_pos] = temp;
            }

            *lastscore += best_delta;
        }
    }
}

// =====================================================
// Local Permutation Optimization
// -----------------------------------------------------
// Looks at a small window of MPERM consecutive candidates
// and tries all possible permutations of that window.
// Keeps the best ordering that maximizes partial score.
//
// This is a brute-force local improvement inside a small
// subrange, which helps escape small local minima.
// =====================================================
static void local_permute(RanksFile *rf, int *perm, double *lastscore, int lo, int hi) {
    int len = hi - lo + 1;
    if (len > MPERM) len = MPERM;

    double bestscore = compute_partial_score(rf, perm, lo, hi);
    double oldscore = bestscore;

    int *best = malloc(len * sizeof(int));
    memcpy(best, &perm[lo], len * sizeof(int));

    int *p = malloc(len * sizeof(int));
    memcpy(p, &perm[lo], len * sizeof(int));

    int done = 0;
    while (!done) {
        // Compute score of current local permutation
        double s = compute_partial_score(rf, perm, lo, hi);
        if (s > bestscore) {
            bestscore = s;
            memcpy(best, &perm[lo], len * sizeof(int));
        }

        // Generate next permutation using next_permutation logic
        int k = len - 2;
        while (k >= 0 && perm[lo + k] > perm[lo + k + 1]) k--;
        if (k < 0) done = 1;
        else {
            int l = len - 1;
            while (perm[lo + k] > perm[lo + l]) l--;
            swap(&perm[lo + k], &perm[lo + l]);
            for (int a = k + 1, b = len - 1; a < b; a++, b--)
                swap(&perm[lo + a], &perm[lo + b]);
        }
    }

    // Apply best found local permutation if improved
    if (bestscore > oldscore) {
        *lastscore += (bestscore - oldscore);
        memcpy(&perm[lo], best, len * sizeof(int));
    }

    free(best);
    free(p);
}

// =====================================================
// Initialize Ranking by Mean Preference
// -----------------------------------------------------
// Creates an initial ordering of candidates using a simple
// heuristic: candidates with fewer "losses" appear earlier.
//
// Specifically, for each candidate i, sum up how many others
// prefer them over i (rf->prefmat[k][i]), then sort by this value.
// =====================================================
static void init_ranking(RanksFile *rf, int *perm) {
    int n = rf->ncands;
    double *score = calloc(n, sizeof(double));

    for (int i = 0; i < n; i++) {
        double s = 0.0;
        for (int k = 0; k < n; k++) {
            if (i == k) continue;
            s += rf->prefmat[k][i];  // Higher means i loses more often
        }
        score[i] = s;
    }

    // Sort by increasing score (fewer losses first)
    for (int i = 0; i < n; i++) perm[i] = i;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (score[perm[j]] > score[perm[j + 1]]) {
                swap(&perm[j], &perm[j + 1]);
            }
        }
    }

    free(score);
}

// =====================================================
// Main Heuristic Kemeny Computation
// -----------------------------------------------------
// This is the main function that coordinates the heuristic:
//   1. Start with an initial ranking (init_ranking)
//   2. Iteratively apply move-insert and local permutation
//   3. Stop when no further improvement is possible
//
// Outputs the final ranking and heuristic Kemeny score.
// =====================================================
void compute_heuristic_kemeny(RanksFile *rf, FILE *outfile) {
    int n = rf->ncands;
    int *perm = malloc(n * sizeof(int));

    // Step 1: Initialize with a simple mean-preference ranking
    init_ranking(rf, perm);

    // Step 2: Compute initial score
    double oldscore = compute_score(rf, perm);

    // Step 3: Iteratively improve the ranking
    for (;;) {
        move_insert(rf, perm, &oldscore);

        // Apply local optimization on small windows
        for (int i = 0; i <= n - MPERM; i++)
            local_permute(rf, perm, &oldscore, i, i + MPERM - 1);

        double newscore = compute_score(rf, perm);

        // If no improvement, stop
        if (newscore <= oldscore)
            break;

        oldscore = newscore;
    }

    // Step 4: Output the final ranking
    fprintf(outfile, "\nHeuristic Kemeny ranking (score = %.0f): ", oldscore / 2.0);
    for (int i = 0; i < n; i++) {
        fprintf(outfile, "%s ", rf->unnames[perm[i]]);
    }
    fprintf(outfile, "\n");

    free(perm);
}
