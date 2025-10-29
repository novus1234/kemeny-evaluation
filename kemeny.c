#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ranksfile.h"

//----------------------------------------------------------
// Helper function: get_candidate_index
//----------------------------------------------------------
// Given a candidate's name (string), find its index in rf->unnames.
// If not found, returns -1.
//----------------------------------------------------------
int get_candidate_index(char *name, RanksFile *rf) {
    for (int i = 0; i < rf->ncands; i++) {
        if (strcmp(name, rf->unnames[i]) == 0) {   // Compare with each stored candidate name
            return i;  // Found → return its index
        }
    }
    return -1; // Not found
}


//----------------------------------------------------------
// Function: read_ranks_file
//----------------------------------------------------------
// Reads voter rankings from an input file.
// Populates the RanksFile struct with candidate names and
// updates the pairwise preference matrix.
//
// Parameters:
// - infile: input stream (e.g., stdin or file)
// - rf: pointer to RanksFile struct to fill
// - outfile: output stream (for printing info)
// - showinput: if nonzero, prints each input line read
//----------------------------------------------------------
void read_ranks_file(FILE *infile, RanksFile *rf, FILE *outfile, int showinput) {
    // Initialize counts
    rf->nrankers = 0;
    rf->ncands = 0;
    rf->nprefs = 0;

    // Read input lines until EOF or max voters reached
    while (fgets(rf->thedata[rf->nrankers], BUFFLEN, infile) != NULL) {
        if (rf->nrankers >= MAXVOTERS) break;   // Stop if too many voters

        // Optionally print the line read
        if (showinput)
            fprintf(outfile, "%s", rf->thedata[rf->nrankers]);

        // Prepare to parse tokens (candidate names)
        char *tokens[MAXCANDS];
        int onevote[MAXCANDS];
        int nvotes = 0;   // Number of candidates ranked in this line

        char *line = rf->thedata[rf->nrankers];
        char *token = strtok(line, " \t\r\n");   // Split by space/tab/newline

        // Process each token (candidate name)
        while (token != NULL) {
            int idx = get_candidate_index(token, rf); // Get candidate index if known

            if (idx == -1) { // New candidate (not seen before)
                idx = rf->ncands;
                if (idx >= MAXCANDS) { // Safety check
                    fprintf(stderr, "Exceeded maximum candidates!\n");
                    exit(1);
                }
                strcpy(rf->unnames[idx], token); // Store candidate name
                rf->ncands++;                    // Increase candidate count
            }

            onevote[nvotes++] = idx;  // Store candidate index for this ranking
            token = strtok(NULL, " \t\r\n"); // Move to next token
        }

        //--------------------------------------------------
        // Update preference matrix based on ranking
        //--------------------------------------------------
        // For each pair (i,j) where i is ranked before j,
        // increment prefmat[i][j] (i preferred to j)
        // and decrement prefmat[j][i].
        //--------------------------------------------------
        for (int i = 0; i < nvotes - 1; i++) {
            for (int j = i + 1; j < nvotes; j++) {
                rf->prefmat[onevote[i]][onevote[j]] += 1;  // i preferred over j
                rf->prefmat[onevote[j]][onevote[i]] -= 1;  // j less preferred than i
                rf->nprefs++;                               // Count total pairwise prefs
            }
        }

        rf->nrankers++; // One voter processed
        if (rf->nrankers >= MAXVOTERS) break; // Safety check
    }

    // Print summary
    fprintf(outfile, "*** There are %d candidates and %d voters. ***\n", rf->ncands, rf->nrankers);
}


//----------------------------------------------------------
// main function
//----------------------------------------------------------
// Entry point. Reads input, builds preference matrix,
// and prints it for verification.
//----------------------------------------------------------
int main(int argc, char **argv) {
    FILE *INP = stdin;   // Default input from standard input
    FILE *OUTP = stdout; // Default output to standard output

    int showinput = 0;   // Whether to print input lines (disabled by default)

    RanksFile rf;                // Create a RanksFile object
    memset(&rf, 0, sizeof(RanksFile)); // Initialize it to zero

    // Read and process all input data
    read_ranks_file(INP, &rf, OUTP, showinput);
    compute_kemeny_bruteforce(&rf, stdout); // call the new function

    //--------------------------------------------------
    // Print the preference matrix for verification
    //--------------------------------------------------
    fprintf(OUTP, "\nPreference matrix:\n");
    for (int i = 0; i < rf.ncands; i++) {
        for (int j = 0; j < rf.ncands; j++) {
            fprintf(OUTP, "%4d ", rf.prefmat[i][j]); // Print each matrix entry
        }
        fprintf(OUTP, "\n");
    }

    return 0; // Program completed successfully
}
