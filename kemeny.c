#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ranksfile.h"

void compute_copeland_approximation(RanksFile *rf, FILE *outfile);

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

/* ---------- Ranked Pairs (Tideman) ---------- */
/* Uses rf->prefmat as signed pairwise margins: rf->prefmat[i][j] > 0 means i beats j. */

typedef struct {
  int from;
  int to;
  int margin; /* positive if from preferred to to */
  int votes;  /* raw positive count used for tie-break (same as margin here) */
} RP_Edge;

/* comparator: strongest margin first, then higher votes, then deterministic index tie-break */
static int rp_cmp_edge(const void *pa, const void *pb) {
  const RP_Edge *a = (const RP_Edge *)pa;
  const RP_Edge *b = (const RP_Edge *)pb;
  if (a->margin != b->margin) return b->margin - a->margin;
  if (a->votes  != b->votes)  return b->votes  - a->votes;
  if (a->from   != b->from)    return a->from   - b->from;
  return a->to - b->to;
}

/* DFS to test whether a path exists start -> target in locked graph */
static int rp_has_path(int **locked, int start, int target, int ncands, int *visited) {
  if (start == target) return 1;
  visited[start] = 1;
  for (int v = 0; v < ncands; ++v) {
    if (locked[start][v] && !visited[v]) {
      if (rp_has_path(locked, v, target, ncands, visited)) return 1;
    }
  }
  return 0;
}

void compute_ranked_pairs(RanksFile *rf, FILE *outfile) {
  if (!rf || !outfile) return;
  int m = rf->ncands;
  if (m <= 0) {
    fprintf(outfile, "No candidates for Ranked Pairs\n");
    return;
  }

  /* Build directed edges for every positive margin rf->prefmat[i][j] > 0 */
  int maxEdges = m * (m - 1);
  RP_Edge *edges = malloc(sizeof(RP_Edge) * maxEdges);
  if (!edges) { fprintf(stderr, "malloc failed\n"); return; }
  int ecount = 0;
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < m; ++j) {
      if (i == j) continue;
      int margin = rf->prefmat[i][j];
      if (margin > 0) {
        edges[ecount].from = i;
        edges[ecount].to = j;
        edges[ecount].margin = margin;
        edges[ecount].votes = margin; /* margin already signed count */
        ecount++;
      }
    }
  }

  /* Sort edges strongest-first */
  qsort(edges, ecount, sizeof(RP_Edge), rp_cmp_edge);

  /* Allocate locked adjacency matrix and init to 0 */
  int **locked = malloc(sizeof(int*) * m);
  if (!locked) { free(edges); fprintf(stderr, "malloc failed\n"); return; }
  for (int i = 0; i < m; ++i) {
    locked[i] = calloc(m, sizeof(int));
    if (!locked[i]) {
      for (int k = 0; k < i; ++k) free(locked[k]);
      free(locked); free(edges);
      fprintf(stderr, "calloc failed\n"); return;
    }
  }

  /* Lock edges greedily, skipping those that create cycles */
  for (int ei = 0; ei < ecount; ++ei) {
    int u = edges[ei].from;
    int v = edges[ei].to;
    int *visited = calloc(m, sizeof(int));
    if (!visited) { fprintf(stderr, "calloc failed\n"); break; }
    /* if there is already a path v -> u, locking u->v would create a cycle */
    int creates = rp_has_path(locked, v, u, m, visited);
    free(visited);
    if (!creates) locked[u][v] = 1;
  }

  /* Topological sort (Kahn's algorithm) to get final ranking */
  int *indeg = calloc(m, sizeof(int));
  if (!indeg) { fprintf(stderr, "calloc failed\n"); }
  else {
    for (int u = 0; u < m; ++u)
      for (int v = 0; v < m; ++v)
        if (locked[u][v]) indeg[v]++;

    int *queue = malloc(sizeof(int) * m);
    int qh = 0, qt = 0;
    for (int i = 0; i < m; ++i) if (indeg[i] == 0) queue[qt++] = i;

    int *order = malloc(sizeof(int) * m);
    int idx = 0;
    while (qh < qt) {
      int u = queue[qh++];
      order[idx++] = u;
      for (int v = 0; v < m; ++v) {
        if (locked[u][v]) {
          indeg[v]--;
          if (indeg[v] == 0) queue[qt++] = v;
        }
      }
    }

    /* If not all nodes included (shouldn't happen), append the rest deterministically */
    if (idx != m) {
      for (int i = 0; i < m; ++i) {
        int found = 0;
        for (int j = 0; j < idx; ++j) if (order[j] == i) { found = 1; break; }
        if (!found) order[idx++] = i;
      }
    }

    /* Print Ranked Pairs result (best first) */
    fprintf(outfile, "\nRanked Pairs (Tideman) ranking:\n");
    for (int k = 0; k < m; ++k) {
      if (k) fprintf(outfile, " > ");
      fprintf(outfile, "%s", rf->unnames[order[k]]);
    }
    fprintf(outfile, "\n");

    free(order);
    free(queue);
    free(indeg);
  }

  /* cleanup */
  for (int i = 0; i < m; ++i) free(locked[i]);
  free(locked);
  free(edges);
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
    compute_copeland_approximation(&rf, stdout); 
    compute_ranked_pairs(&rf, stdout);   /* <-- added call for Ranked Pairs */

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
