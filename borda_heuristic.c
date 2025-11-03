#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ranksfile.h"

// Compute an approximate Kemeny consensus using the Borda Count heuristic
void compute_borda_heuristic(RanksFile *rf, FILE *outfile) {
    int n = rf->ncands;
    int m = rf->nrankers;

    if (n == 0 || m == 0) {
        fprintf(outfile, "No data available to compute heuristic.\n");
        return;
    }

    // Array for storing Borda scores
    double scores[MAXCANDS] = {0};

    // Compute scores using the preference matrix
    // Each candidate gets 1 point for every other candidate it beats
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j && rf->prefmat[i][j] > 0) {
                scores[i] += rf->prefmat[i][j];
            }
        }
    }

    // Array to keep candidate indices
    int ranking[MAXCANDS];
    for (int i = 0; i < n; i++) ranking[i] = i;

    // Sort candidates by descending Borda score (simple bubble sort)
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (scores[ranking[j]] > scores[ranking[i]]) {
                int tmp = ranking[i];
                ranking[i] = ranking[j];
                ranking[j] = tmp;
            }
        }
    }

    // Output results
    fprintf(outfile, "\nBorda Count Heuristic Ranking:\n");
    for (int i = 0; i < n; i++) {
        fprintf(outfile, "%d. %s (score = %.2f)\n", i + 1,
                rf->unnames[ranking[i]], scores[ranking[i]]);
    }
}
