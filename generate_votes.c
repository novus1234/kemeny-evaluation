#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Fisher-Yates shuffle for random permutations
void shuffle(int *array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_candidates> <num_voters>\n", argv[0]);
        return 1;
    }

    int num_cands = atoi(argv[1]);
    int num_voters = atoi(argv[2]);

    if (num_cands <= 0 || num_voters <= 0) {
        fprintf(stderr, "Both inputs must be positive integers.\n");
        return 1;
    }

    FILE *fp = fopen("votes.txt", "w");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    srand((unsigned int)time(NULL));

    int *ranking = malloc(num_cands * sizeof(int));
    if (!ranking) {
        perror("Memory allocation failed");
        fclose(fp);
        return 1;
    }

    // Write random rankings for each voter
    for (int v = 0; v < num_voters; v++) {
        for (int i = 0; i < num_cands; i++) {
            ranking[i] = i + 1;  // Candidate IDs: 1, 2, 3, ...
        }
        shuffle(ranking, num_cands);

        for (int i = 0; i < num_cands; i++) {
            fprintf(fp, "%d", ranking[i]);
            if (i != num_cands - 1)
                fprintf(fp, " ");
        }
        fprintf(fp, "\n");
    }

    printf("Generated votes.txt with %d voters and %d candidates.\n", num_voters, num_cands);

    free(ranking);
    fclose(fp);
    return 0;
}
