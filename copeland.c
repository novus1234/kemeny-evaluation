#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ranksfile.h"

//----------------------------------------------------------
// Comparison function for qsort (descending order)
//----------------------------------------------------------
int compare_candidates(const void *a, const void *b, void *scores_ptr) {
    float *scores = (float *)scores_ptr;
    int idx_a = *(int *)a;
    int idx_b = *(int *)b;
    
    // Sort in descending order
    if (scores[idx_a] > scores[idx_b]) return -1;
    if (scores[idx_a] < scores[idx_b]) return 1;
    return 0;
}

//----------------------------------------------------------
// Function: compute_copeland_approximation
//----------------------------------------------------------
void compute_copeland_approximation(RanksFile *rf, FILE *outfile) {
    fprintf(outfile, "\n=== COPELAND APPROXIMATION ===\n");
    
    int n = rf->ncands;
    float scores[MAXCANDS];
    int indices[MAXCANDS];
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        scores[i] = 0.0;
        indices[i] = i;
    }
    
    // Build win/loss/tie matrix and calculate scores
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j) {
                if (rf->prefmat[i][j] > 0) {
                    scores[i] += 1.0;  // win
                } else if (rf->prefmat[i][j] == 0) {
                    scores[i] += 0.5;  // tie
                }
                // loss adds 0.0
            }
        }
    }
    
    // Print Copeland scores
    fprintf(outfile, "Copeland scores (wins + 0.5*ties):\n");
    for (int i = 0; i < n; i++) {
        fprintf(outfile, "%s: %.1f\n", rf->unnames[i], scores[i]);
    }
    
    // Use qsort for fast sorting (O(n log n))
    qsort_r(indices, n, sizeof(int), compare_candidates, scores);
    
    // Print final ranking
    fprintf(outfile, "\nFinal Copeland Ranking:\n");
    for (int i = 0; i < n; i++) {
        int idx = indices[i];
        fprintf(outfile, "%d. %s (score: %.1f)\n", i + 1, rf->unnames[idx], scores[idx]);
    }
}