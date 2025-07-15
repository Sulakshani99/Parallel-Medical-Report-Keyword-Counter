#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEYWORDS 100
#define MAX_LINE_LEN 1000
#define MAX_KEYWORD_LENGTH 50

int main(int argc, char** argv) {

    int num_threads = 4;
    omp_set_num_threads(num_threads);

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char keywords[MAX_KEYWORDS][MAX_KEYWORD_LENGTH];
    int totalKeywords = 0;
    int keywordCount[MAX_KEYWORDS] = {0};

    int totalLines = 0;
    char **allLines = NULL;
    int *sendcounts = NULL, *displs = NULL;

    double t1 = 0, t2 = 0;

    // Step 1: Process 0 reads keywords and reports
    if (rank == 0) {
        FILE *kf = fopen("../keywords.txt", "r");
        if (!kf) {
            printf("Error: Cannot open keywords.txt\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        char buffer[MAX_KEYWORD_LENGTH];
        while (fgets(buffer, MAX_KEYWORD_LENGTH, kf)) {
            buffer[strcspn(buffer, "\n")] = 0;
            strcpy(keywords[totalKeywords++], buffer);
        }
        fclose(kf);

        FILE *rf = fopen("../data/medical_reports.txt", "r");
        if (!rf) {
            printf("Error: Cannot open medical_reports.txt\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
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

    // Step 2: Broadcast keywords and total line count
    MPI_Bcast(&totalKeywords, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(keywords, MAX_KEYWORDS * MAX_KEYWORD_LENGTH, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&totalLines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Step 3: Prepare sendcounts and displs for MPI_Scatterv
    int myLinesCount = 0;
    if (rank == 0) {
        sendcounts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
        int base = totalLines / size, rem = totalLines % size;
        int offset = 0;
        for (int i = 0; i < size; i++) {
            sendcounts[i] = base + (i < rem ? 1 : 0);
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }
    if (rank != 0) sendcounts = malloc(size * sizeof(int));
    MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
    myLinesCount = sendcounts[rank];

    // Step 4: Prepare receive buffer
    char *recvbuf = malloc(myLinesCount * MAX_LINE_LEN);
    char *sendbuf = NULL;
    if (rank == 0) {
        sendbuf = malloc(totalLines * MAX_LINE_LEN);
        for (int i = 0; i < totalLines; i++) {
            strncpy(sendbuf + i * MAX_LINE_LEN, allLines[i], MAX_LINE_LEN);
        }
    }
    int *displs_bytes = NULL, *sendcounts_bytes = NULL;
    if (rank == 0) {
        displs_bytes = malloc(size * sizeof(int));
        sendcounts_bytes = malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            sendcounts_bytes[i] = sendcounts[i] * MAX_LINE_LEN;
            displs_bytes[i] = displs[i] * MAX_LINE_LEN;
        }
    }
    MPI_Scatterv(sendbuf, sendcounts_bytes, displs_bytes, MPI_CHAR,
                 recvbuf, myLinesCount * MAX_LINE_LEN, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i = 0; i < totalLines; i++) free(allLines[i]);
        free(allLines);
        free(sendbuf);
        free(displs);
        free(sendcounts_bytes);
        free(displs_bytes);
    }

    int localCount[MAX_KEYWORDS] = {0};
    t1 = MPI_Wtime();

    // Step 5: Hybrid parallel keyword counting
    #pragma omp parallel for
    for (int i = 0; i < myLinesCount; i++) {
        for (int k = 0; k < totalKeywords; k++) {
            if (strstr(recvbuf + i * MAX_LINE_LEN, keywords[k])) {
                #pragma omp atomic
                localCount[k]++;
            }
        }
    }
    t2 = MPI_Wtime();

    // Step 6: Reduce results
    MPI_Reduce(localCount, keywordCount, totalKeywords, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Step 7: Output results
    if (rank == 0) {
        FILE *out = fopen("../outputs/result_hybrid.txt", "w");
        for (int i = 0; i < totalKeywords; i++) {
            fprintf(out, "%s: %d\n", keywords[i], keywordCount[i]);
        }
        fclose(out);

        FILE *perf = fopen("../outputs/performance.txt", "a");
        fprintf(perf, "Hybrid version time: %.6f seconds\t No. of Processes: %d\t No. of Threads: %d\n", t2 - t1, size, num_threads);
        fclose(perf);

        printf("Hybrid Done in %.6f sec using %d MPI processes × %d OpenMP threads\n", t2 - t1, size, num_threads);
    }

    free(recvbuf);
    free(sendcounts);
    MPI_Finalize();
    
    return 0;
}

