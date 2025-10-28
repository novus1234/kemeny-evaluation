#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ranksfile.h"  // we'll make a header file with RanksFile struct

//-----------------------------------------------------
// Helper: compute Kemeny score for a given ranking
//-----------------------------------------------------
static int compute_kemeny_score(int *ranking, RanksFile *rf) {
    int score = 0;
    for (int i = 0; i < rf->ncands; i++) {
        for (int j = i + 1; j < rf->ncands; j++) {
            int a = ranking[i];
            int b = ranking[j];
            score += rf->prefmat[a][b]; // Add preference difference
        }
    }
    return score;
}

//-----------------------------------------------------
// Helper: swap two integers (used for permutations)
//-----------------------------------------------------
static void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

//-----------------------------------------------------
// Recursive permutation generator for brute force search
//-----------------------------------------------------
static void permute(RanksFile *rf, int *arr, int l, int r, int *best_score, int *best_perm) {
    if (l == r) {
        int score = compute_kemeny_score(arr, rf);
        if (score > *best_score) {
            *best_score = score;
            memcpy(best_perm, arr, rf->ncands * sizeof(int));
        }
        return;
    }

    for (int i = l; i <= r; i++) {
        swap(&arr[l], &arr[i]);
        permute(rf, arr, l + 1, r, best_score, best_perm);
        swap(&arr[l], &arr[i]); // backtrack
    }
}

//-----------------------------------------------------
// Main callable function
//-----------------------------------------------------
// Takes a filled RanksFile struct, computes the
// best Kemeny consensus ranking using brute force.
// Prints the result directly.
//-----------------------------------------------------
void compute_kemeny_bruteforce(RanksFile *rf, FILE *out) {
    if (rf->ncands > 10) {
        fprintf(out, "Too many candidates (%d). Brute force limited to <= 10.\n", rf->ncands);
        return;
    }

    int arr[rf->ncands];
    for (int i = 0; i < rf->ncands; i++)
        arr[i] = i;

    int best_score = -999999;
    int best_perm[rf->ncands];

    fprintf(out, "\nComputing Kemeny consensus (brute force)...\n");
    permute(rf, arr, 0, rf->ncands - 1, &best_score, best_perm);

    fprintf(out, "\nBest Kemeny score: %d\nBest ranking: ", best_score);
    for (int i = 0; i < rf->ncands; i++) {
        fprintf(out, "%s ", rf->unnames[best_perm[i]]);
    }
    fprintf(out, "\n");
}
