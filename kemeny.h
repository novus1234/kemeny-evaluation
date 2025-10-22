#ifndef KEMENY_H
#define KEMENY_H

#define MAX_CANDIDATES 20
#define MAX_VOTERS 200

typedef struct {
    int num_candidates;
    int num_voters;
    int rankings[MAX_VOTERS][MAX_CANDIDATES];
    int pairwise[MAX_CANDIDATES][MAX_CANDIDATES];
} Election;

// ---------- IO and Matrix Setup ---------- //
int load_preferences(Election *e, const char *filename);
void compute_pairwise(Election *e);

// ---------- Algorithms ---------- //
void compute_exact_kemeny(Election *e);

#endif
