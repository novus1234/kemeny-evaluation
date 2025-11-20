#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ranksfile.h"

//----------------------------------------------------------
// Function: compute_quicksort_approximation
//----------------------------------------------------------
// Quicksort approximation for Kemeny:
// 1. Use majority rule comparisons to sort candidates
// 2. The sorted order is the consensus ranking
//----------------------------------------------------------
void compute_quicksort_approximation(RanksFile *rf, FILE *outfile) {
    fprintf(outfile, "\n=== QUICKSORT APPROXIMATION ===\n");
    
    int n = rf->ncands;
    int indices[MAXCANDS];
    
    // Initialize indices array
    for (int i = 0; i < n; i++) {
        indices[i] = i;
    }
    
    // Simple bubble sort using majority rule comparisons
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            int cand_a = indices[j];
            int cand_b = indices[j + 1];
            
            // Use majority rule: if more voters prefer B over A, swap them
            if (rf->prefmat[cand_b][cand_a] > rf->prefmat[cand_a][cand_b]) {
                // Swap indices
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // Print final ranking
    fprintf(outfile, "\nFinal Quicksort Ranking:\n");
    for (int i = 0; i < n; i++) {
        int idx = indices[i];
        fprintf(outfile, "%d. %s\n", i + 1, rf->unnames[idx]);
    }
}