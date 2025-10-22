#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kemeny.h"

static void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static int kemeny_score(Election *e, int *perm) {
    int score = 0;
    for (int i = 0; i < e->num_candidates; i++) {
        for (int j = i + 1; j < e->num_candidates; j++) {
            int a = perm[i];
            int b = perm[j];
            score += e->pairwise[a][b];
        }
    }
    return score;
}

static void generate_permutations(Election *e, int *arr, int n, int *best_perm, int *best_score) {
    if (n == 1) {
        int score = kemeny_score(e, arr);
        if (score > *best_score) {
            *best_score = score;
            memcpy(best_perm, arr, sizeof(int) * e->num_candidates);
        }
        return;
    }
    for (int i = 0; i < n; i++) {
        generate_permutations(e, arr, n - 1, best_perm, best_score);
        swap(&arr[n % 2 == 1 ? 0 : i], &arr[n - 1]);
    }
}

void compute_exact_kemeny(Election *e) {
    int perm[MAX_CANDIDATES];
    for (int i = 0; i < e->num_candidates; i++)
        perm[i] = i;

    int best_perm[MAX_CANDIDATES];
    int best_score = -1;

    generate_permutations(e, perm, e->num_candidates, best_perm, &best_score);

    printf("Best Kemeny score: %d\nRanking: ", best_score);
    for (int i = 0; i < e->num_candidates; i++)
        printf("%d ", best_perm[i]);
    printf("\n");
}
