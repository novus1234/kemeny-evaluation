#ifndef RANKSFILE_H
#define RANKSFILE_H

// Define constants for array limits
#define MAXVOTERS 3000        // Maximum number of voters (rankers)
#define MAXCANDS 1000         // Maximum number of candidates
#define MAXCANDNAMELEN 64     // Maximum length of candidate names
#define BUFFLEN 1024          // Maximum input line length

// Define a structure to hold all the ranking data
typedef struct {
    char thedata[MAXVOTERS][BUFFLEN];         // Raw input lines (each voter's ranking)
    int nrankers;                             // Number of voters (lines read)
    int ncands;                               // Number of unique candidates
    int nprefs;                               // Number of pairwise preferences recorded
    int prefmat[MAXCANDS][MAXCANDS];          // Preference matrix: prefmat[i][j] counts how many prefer i over j
    char unnames[MAXCANDS][MAXCANDNAMELEN];   // List of candidate names
} RanksFile;

// Declaration of the brute-force Kemeny function
void compute_kemeny_bruteforce(RanksFile *rf, FILE *out);
void compute_copeland_approximation(RanksFile *rf, FILE *out);

#endif
