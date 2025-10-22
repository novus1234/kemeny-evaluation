#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kemeny.h"

int load_preferences(Election *e, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Error opening file");
        return 0;
    }

    char line[256];
    int voter = 0;
    int num_candidates = -1;

    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) <= 1) continue;

        int c = 0;
        char *token = strtok(line, " \t\n");
        while (token && c < MAX_CANDIDATES) {
            e->rankings[voter][c++] = atoi(token);
            token = strtok(NULL, " \t\n");
        }

        if (num_candidates == -1)
            num_candidates = c;
        else if (c != num_candidates) {
            fprintf(stderr, "Error: inconsistent candidate count on line %d\n", voter + 1);
            fclose(f);
            return 0;
        }

        voter++;
        if (voter >= MAX_VOTERS) break;
    }

    fclose(f);
    e->num_voters = voter;
    e->num_candidates = num_candidates;
    return 1;
}

void compute_pairwise(Election *e) {
    memset(e->pairwise, 0, sizeof(e->pairwise));
    for (int v = 0; v < e->num_voters; v++) {
        for (int i = 0; i < e->num_candidates; i++) {
            int ci = e->rankings[v][i];
            for (int j = i + 1; j < e->num_candidates; j++) {
                int cj = e->rankings[v][j];
                e->pairwise[ci][cj]++;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <preferences_file>\n", argv[0]);
        return 1;
    }

    Election e;
    if (!load_preferences(&e, argv[1])) {
        fprintf(stderr, "Failed to load preferences.\n");
        return 1;
    }

    printf("Loaded %d voters, %d candidates.\n", e.num_voters, e.num_candidates);
    compute_pairwise(&e);

    // Run exact Kemeny algorithm
    compute_exact_kemeny(&e);

    return 0;
}
