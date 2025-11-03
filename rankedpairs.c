// rankedpairs.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ranksfile.h"

typedef struct {
    int from;
    int to;
    int margin;
} Edge;

static int has_path(int **locked, int start, int target, int ncands, int *visited) {
    if (start == target) return 1;
    visited[start] = 1;
    for (int v = 0; v < ncands; ++v) {
        if (locked[start][v] && !visited[v]) {
            if (has_path(locked, v, target, ncands, visited)) return 1;
        }
    }
    return 0;
}

static int compare_edges(const void *a, const void *b) {
    const Edge *ea = (const Edge *)a;
    const Edge *eb = (const Edge *)b;
    return eb->margin - ea->margin;
}

void compute_ranked_pairs(RanksFile *rf, FILE *outfile) {
    int m = rf->ncands;
    int max_edges = m * (m - 1);
    Edge *edges = malloc(sizeof(Edge) * max_edges);
    int edge_count = 0;

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            if (i != j && rf->prefmat[i][j] > 0) {
                edges[edge_count++] = (Edge){i, j, rf->prefmat[i][j]};
            }
        }
    }

    qsort(edges, edge_count, sizeof(Edge), compare_edges);

    int **locked = malloc(sizeof(int *) * m);
    for (int i = 0; i < m; ++i) {
        locked[i] = calloc(m, sizeof(int));
    }

    for (int i = 0; i < edge_count; ++i) {
        int u = edges[i].from;
        int v = edges[i].to;
        int *visited = calloc(m, sizeof(int));
        if (!has_path(locked, v, u, m, visited)) {
            locked[u][v] = 1;
        }
        free(visited);
    }

    int *indegree = calloc(m, sizeof(int));
    for (int u = 0; u < m; ++u) {
        for (int v = 0; v < m; ++v) {
            if (locked[u][v]) indegree[v]++;
        }
    }

    int *queue = malloc(sizeof(int) * m);
    int *ranking = malloc(sizeof(int) * m);
    int front = 0, back = 0;

    for (int i = 0; i < m; ++i) {
        if (indegree[i] == 0) queue[back++] = i;
    }

    int idx = 0;
    while (front < back) {
        int u = queue[front++];
        ranking[idx++] = u;
        for (int v = 0; v < m; ++v) {
            if (locked[u][v]) {
                indegree[v]--;
                if (indegree[v] == 0) queue[back++] = v;
            }
        }
    }

    fprintf(outfile, "\nRanked Pairs (Tideman) ranking:\n");
    for (int i = 0; i < m; ++i) {
        if (i > 0) fprintf(outfile, " > ");
        fprintf(outfile, "%s", rf->unnames[ranking[i]]);
    }
    fprintf(outfile, "\n");

    free(edges);
    for (int i = 0; i < m; ++i) free(locked[i]);
    free(locked);
    free(indegree);
    free(queue);
    free(ranking);
}
