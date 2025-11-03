// ranked_pairs.c
// Ranked Pairs (Tideman) algorithm reading votes as lines like:
// Alice Bob Carol
// Bob Carol Alice
// Carol Alice Bob
//
// Each line is a full ranking: leftmost = top preference.
// Compile: gcc -O2 -std=c11 -Wall -Wextra ranked_pairs.c -o ranked_pairs
// Run: ./ranked_pairs < votes.txt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 4096
#define MAX_CAND 200

typedef struct {
    int from;
    int to;
    int margin;
    int votes;
} Edge;

static int m = 0;
static char *names[MAX_CAND];
static int P[MAX_CAND][MAX_CAND]; // pairwise counts

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static void chomp(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
}

static char *ltrim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

static int find_name(const char *name) {
    for (int i = 0; i < m; ++i) if (strcmp(names[i], name) == 0) return i;
    return -1;
}

static int add_name(const char *name) {
    if (m >= MAX_CAND) die("Too many candidates");
    names[m] = strdup(name);
    if (!names[m]) die("strdup failed");
    return m++;
}

// Tokenize a line (in-place). Returns number of tokens and fills tokens[].
static int tokenize(char *line, char **tokens, int max) {
    int nt = 0;
    char *p = line;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        char *start = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (*p) { *p = '\0'; p++; }
        if (nt < max) tokens[nt++] = start;
    }
    return nt;
}

static void read_votes_and_build_matrix(void) {
    char line[MAX_LINE];
    int first = 1;

    while (fgets(line, sizeof(line), stdin)) {
        chomp(line);
        char *s = ltrim(line);
        if (*s == '\0') continue; // skip empty

        char *tokens[MAX_CAND];
        int nt = tokenize(s, tokens, MAX_CAND);
        if (nt == 0) continue;

        // On first vote, use tokens to set candidate list
        if (first) {
            for (int i = 0; i < nt; ++i) {
                if (find_name(tokens[i]) == -1) add_name(tokens[i]);
            }
            first = 0;
        } else {
            // Robustness: if a token name is unseen, add it
            for (int i = 0; i < nt; ++i) {
                if (find_name(tokens[i]) == -1) add_name(tokens[i]);
            }
        }

        // Map tokens to ranks; assume tokens list is ranked highest -> lowest
        int rank[MAX_CAND];
        for (int i = 0; i < m; ++i) rank[i] = m + 5; // large default for unmentioned
        for (int i = 0; i < nt; ++i) {
            int idx = find_name(tokens[i]);
            if (idx >= 0) rank[idx] = i;
        }

        // Update pairwise counts: for each pair i,j, if rank[i] < rank[j] then voter prefers i over j
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) {
                if (i == j) continue;
                if (rank[i] < rank[j]) P[i][j] += 1;
            }
        }
    }
}

static int cmp_edge(const void *a, const void *b) {
    const Edge *ea = (const Edge *)a;
    const Edge *eb = (const Edge *)b;
    if (ea->margin != eb->margin) return eb->margin - ea->margin;
    if (ea->votes != eb->votes) return eb->votes - ea->votes;
    if (ea->from != eb->from) return ea->from - eb->from;
    return ea->to - eb->to;
}

static int has_path_dfs(int **locked, int start, int target, int *vis) {
    if (start == target) return 1;
    vis[start] = 1;
    for (int v = 0; v < m; ++v) {
        if (locked[start][v] && !vis[v]) {
            if (has_path_dfs(locked, v, target, vis)) return 1;
        }
    }
    return 0;
}

int main(void) {
    // init
    m = 0;
    for (int i = 0; i < MAX_CAND; ++i) names[i] = NULL;
    for (int i = 0; i < MAX_CAND; ++i) for (int j = 0; j < MAX_CAND; ++j) P[i][j] = 0;

    read_votes_and_build_matrix();
    if (m == 0) die("No candidates found");

    // Build edges with positive margin
    int maxEdges = m * (m - 1);
    Edge *edges = malloc(maxEdges * sizeof(Edge));
    if (!edges) die("malloc failed");
    int edgeCount = 0;
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            if (i == j) continue;
            int margin = P[i][j] - P[j][i];
            if (margin > 0) {
                edges[edgeCount].from = i;
                edges[edgeCount].to = j;
                edges[edgeCount].margin = margin;
                edges[edgeCount].votes = P[i][j];
                edgeCount++;
            }
        }
    }

    qsort(edges, edgeCount, sizeof(Edge), cmp_edge);

    // locked adjacency matrix
    int **locked = malloc(m * sizeof(int *));
    if (!locked) die("malloc failed");
    for (int i = 0; i < m; ++i) {
        locked[i] = calloc(m, sizeof(int));
        if (!locked[i]) die("calloc failed");
    }

    // Lock edges greedily unless they create a cycle
    for (int e = 0; e < edgeCount; ++e) {
        int u = edges[e].from;
        int v = edges[e].to;
        int *vis = calloc(m, sizeof(int));
        if (!vis) die("calloc failed");
        int creates = has_path_dfs(locked, v, u, vis);
        free(vis);
        if (!creates) locked[u][v] = 1;
    }

    // Kahn topological sort
    int *indeg = calloc(m, sizeof(int));
    if (!indeg) die("calloc failed");
    for (int u = 0; u < m; ++u) for (int v = 0; v < m; ++v) if (locked[u][v]) indeg[v]++;

    int *queue = malloc(m * sizeof(int));
    int qh = 0, qt = 0;
    for (int i = 0; i < m; ++i) if (indeg[i] == 0) queue[qt++] = i;

    int *order = malloc(m * sizeof(int));
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

    if (idx != m) {
        // fallback: output by index
        idx = 0;
        for (int i = 0; i < m; ++i) order[idx++] = i;
    }

    // print names in order
    for (int i = 0; i < m; ++i) {
        if (i) printf(" ");
        printf("%s", names[order[i]]);
    }
    printf("\n");

    // cleanup
    free(edges);
    free(indeg);
    free(queue);
    free(order);
    for (int i = 0; i < m; ++i) {
        free(locked[i]);
        free(names[i]);
    }
    free(locked);
    return 0;
}
