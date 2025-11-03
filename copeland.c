#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ranksfile.h"

//----------------------------------------------------------
// Structure to hold scores for sorting
//----------------------------------------------------------
typedef struct {
    int index;
    float score;
} CandidateScore;

//----------------------------------------------------------
// Comparison function for qsort (descending order)
//----------------------------------------------------------
int compare_candidates(const void *a, const void *b) {
    const CandidateScore *score_a = (const CandidateScore *)a;
    const CandidateScore *score_b = (const CandidateScore *)b;

    // Sort in descending order
    if (score_a->score > score_b->score) return -1;
    if (score_a->score < score_b->score) return 1;
    return 0;
}

//----------------------------------------------------------
// Function: compute_copeland_approximation
//----------------------------------------------------------
void compute_copeland_approximation(RanksFile *rf, FILE *outfile) {
    fprintf(outfile, "\n=== COPELAND APPROXIMATION ===\n");

    int n = rf->ncands;
    CandidateScore candidates[MAXCANDS];

    // Initialize scores
    for (int i = 0; i < n; i++) {
        candidates[i].index = i;
        candidates[i].score = 0.0;
    }

    // Build win/loss/tie matrix and calculate scores
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j) {
                if (rf->prefmat[i][j] > 0) {
                    candidates[i].score += 1.0;  // win
                } else if (rf->prefmat[i][j] == 0) {
                    candidates[i].score += 0.5;  // tie
                }
                // loss adds 0.0
            }
        }
    }

    // Print Copeland scores
    fprintf(outfile, "Copeland scores (wins + 0.5*ties):\n");
    for (int i = 0; i < n; i++) {
        fprintf(outfile, "%s: %.1f\n", rf->unnames[i], candidates[i].score);
    }

    // Use qsort for fast sorting (O(n log n))
    qsort(candidates, n, sizeof(CandidateScore), compare_candidates);

    // Print final ranking
    fprintf(outfile, "\nFinal Copeland Ranking:\n");
    for (int i = 0; i < n; i++) {
        fprintf(outfile, "%d. %s (score: %.1f)\n", i + 1,
                rf->unnames[candidates[i].index], candidates[i].score);
    }
}
