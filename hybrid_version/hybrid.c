#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEYWORDS 100
#define MAX_LINE_LEN 1000

int main(int argc, char** argv) {

    int num_threads = 4;
    omp_set_num_threads(num_threads); 	

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char keywords[MAX_KEYWORDS][50];
    int totalKeywords = 0;
    int keywordCount[MAX_KEYWORDS] = {0};

    int totalLines = 0;
    char **allLines = NULL;

    // Only rank 0 reads files
    if (rank == 0) {
        FILE *kf = fopen("../keywords.txt", "r");
        while (fgets(keywords[totalKeywords], 50, kf)) {
            keywords[totalKeywords][strcspn(keywords[totalKeywords], "\n")] = 0;
            totalKeywords++;
        }
        fclose(kf);

        FILE *rf = fopen("../data/medical_reports.txt", "r");
        size_t cap = 100000;
        allLines = malloc(cap * sizeof(char*));
        char temp[MAX_LINE_LEN];
        while (fgets(temp, MAX_LINE_LEN, rf)) {
            if (totalLines >= cap) {
                cap *= 2;
                allLines = realloc(allLines, cap * sizeof(char*));
            }
            allLines[totalLines++] = strdup(temp);
        }
        fclose(rf);
    }

    // Broadcast data to all processes
    MPI_Bcast(&totalKeywords, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(keywords, MAX_KEYWORDS * 50, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&totalLines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int linesPerProcess = totalLines / size;
    int start = rank * linesPerProcess;
    int end = (rank == size - 1) ? totalLines : start + linesPerProcess;
    int myLinesCount = end - start;

    char **myLines = malloc(myLinesCount * sizeof(char*));
    for (int i = 0; i < myLinesCount; i++) {
        myLines[i] = malloc(MAX_LINE_LEN);
    }

    if (rank == 0) {
        for (int r = 1; r < size; r++) {
            int s = r * linesPerProcess;
            int e = (r == size - 1) ? totalLines : s + linesPerProcess;
            for (int i = s; i < e; i++) {
                MPI_Send(allLines[i], MAX_LINE_LEN, MPI_CHAR, r, 0, MPI_COMM_WORLD);
            }
        }
        for (int i = 0; i < myLinesCount; i++) {
            strncpy(myLines[i], allLines[start + i], MAX_LINE_LEN);
        }
    } else {
        for (int i = 0; i < myLinesCount; i++) {
            MPI_Recv(myLines[i], MAX_LINE_LEN, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    double t1 = MPI_Wtime();

    int localCount[MAX_KEYWORDS] = {0};

    // Hybrid part: use OpenMP threads in each MPI process
    #pragma omp parallel for
    for (int i = 0; i < myLinesCount; i++) {
        for (int k = 0; k < totalKeywords; k++) {
            if (strstr(myLines[i], keywords[k])) {
                #pragma omp atomic
                localCount[k]++;
            }
        }
    }

    MPI_Reduce(localCount, keywordCount, totalKeywords, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    double t2 = MPI_Wtime();

    if (rank == 0) {
        FILE *out = fopen("../outputs/result_hybrid.txt", "w");
        for (int i = 0; i < totalKeywords; i++) {
            fprintf(out, "%s: %d\n", keywords[i], keywordCount[i]);
        }
        fclose(out);

        FILE *perf = fopen("../outputs/performance.txt", "a");
        fprintf(perf, "Hybrid version time (%d processes × OpenMP): %.6f sec\n", size, t2 - t1);
        fclose(perf);

        printf("Hybrid Done in %.6f sec using %d MPI processes + OpenMP threads\n", t2 - t1, size);

        for (int i = 0; i < totalLines; i++) free(allLines[i]);
        free(allLines);
    }

    for (int i = 0; i < myLinesCount; i++) free(myLines[i]);
    free(myLines);

    MPI_Finalize();
    return 0;
}
