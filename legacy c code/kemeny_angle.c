#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ranksfile.h"

// ---------- Find index ----------
int find_index(int *arr, int n, int x) {
    for (int i = 0; i < n; i++) if (arr[i] == x) return i;
    return -1;
}

// ---------- φ(σ) ----------
void phi_sigma(int *sigma, int n, double *phi) {
    int idx = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++, idx++)
            phi[idx] = (find_index(sigma, n, i) < find_index(sigma, n, j)) ? 1.0 : -1.0;
}

// ---------- φ(D_N) ----------
void phi_dataset(RanksFile *rf, double *phi) {
    int n = rf->ncands;
    int idx = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++, idx++)
            phi[idx] = rf->prefmat[i][j] - rf->prefmat[j][i];
}

// ---------- Cosine similarity ----------
double cosine_similarity(double *a, double *b, int len) {
    double dot = 0, na = 0, nb = 0;
    for (int i = 0; i < len; i++) {
        dot += a[i] * b[i];
        na += a[i] * a[i];
        nb += b[i] * b[i];
    }
    return dot / (sqrt(na) * sqrt(nb));
}

// ---------- Read votes.txt ----------
int read_votes(const char *filename, RanksFile *rf) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;

    rf->nrankers = 0;
    rf->ncands = 0;
    char line[BUFFLEN];

    while (fgets(line, sizeof(line), f)) {
        if (rf->nrankers >= MAXVOTERS) break;
        strcpy(rf->thedata[rf->nrankers], line);

        int pos = 0;
        char temp[BUFFLEN];
        strcpy(temp, line);
        char *token = strtok(temp, " \t\n");
        while (token) {
            int val = atoi(token) - 1; // 0-based
            if (val+1 > rf->ncands) rf->ncands = val+1;
            token = strtok(NULL, " \t\n");
            pos++;
        }
        rf->nrankers++;
    }
    fclose(f);

    // Build pairwise preference matrix
    memset(rf->prefmat, 0, sizeof(rf->prefmat));
    for (int v = 0; v < rf->nrankers; v++) {
        int rank[MAXCANDS];
        int pos = 0;
        char temp[BUFFLEN];
        strcpy(temp, rf->thedata[v]);
        char *token = strtok(temp, " \t\n");
        while (token && pos < rf->ncands) {
            rank[pos++] = atoi(token) - 1;
            token = strtok(NULL, " \t\n");
        }
        for (int i = 0; i < rf->ncands; i++)
            for (int j = i + 1; j < rf->ncands; j++)
                rf->prefmat[rank[i]][rank[j]]++;
    }

    return 1;
}

// ---------- Main ----------
int main() {
    RanksFile rf;
    memset(&rf, 0, sizeof(RanksFile));

    if (!read_votes("votes.txt", &rf)) {
        fprintf(stderr, "Failed to read votes.txt\n");
        return 1;
    }

    printf("Loaded %d voters, %d candidates.\n", rf.nrankers, rf.ncands);

    // Get user guess
    int guess[MAXCANDS];
    printf("Enter your guessed ranking (space-separated, 1..%d): ", rf.ncands);
    for (int i = 0; i < rf.ncands; i++)
        scanf("%d", &guess[i]);

    // Convert to 0-based internally
    for (int i = 0; i < rf.ncands; i++) guess[i]--;

    int n = rf.ncands;
    int npairs = n * (n - 1) / 2;
    double phi_guess[npairs], phi_data[npairs];

    phi_sigma(guess, n, phi_guess);
    phi_dataset(&rf, phi_data);

    double cos_theta = cosine_similarity(phi_guess, phi_data, npairs);
    printf("cos(theta_N(sigma)) = %.4f\n", cos_theta);

    // Theorem bound: find maximum k satisfying cos(theta) > sqrt(1 - (k+1)/C(n,2))
    int maxPairs = npairs;
    int k_found = -1;
    for (int k = 0; k < maxPairs; k++) {
        double rhs = sqrt(1.0 - (double)(k + 1) / maxPairs);
        if (cos_theta > rhs) {
            k_found = k;
            break;
        }
    }

    if (k_found >= 0)
        printf("⇒ Theorem: Your guess is at most %d inversions away from Kemeny consensus\n", k_found);
    else
        printf("⇒ Theorem: Condition not satisfied, guess may be far from consensus\n");

    return 0;
}
