#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_KEYWORDS 100
#define MAX_LINE_LEN 1000

int main() {

    int num_threads = 4;
    omp_set_num_threads(num_threads); 	

    FILE *reportFile = fopen("../data/medical_reports.txt", "r");
    FILE *keywordFile = fopen("../keywords.txt", "r");

    if (!reportFile || !keywordFile) {
        printf("Cannot open input files\n");
        return 1;
    }

    // Load keywords
    char keywords[MAX_KEYWORDS][50];
    int keywordCount[MAX_KEYWORDS] = {0};
    int totalKeywords = 0;
    char buffer[50];

    while (fgets(buffer, 50, keywordFile)) {
        buffer[strcspn(buffer, "\n")] = 0;
        strcpy(keywords[totalKeywords++], buffer);
    }

    // Dynamically read all reports
    size_t capacity = 100000;
    size_t totalReports = 0;
    char **reports = malloc(capacity * sizeof(char*));
    char line[MAX_LINE_LEN];

    while (fgets(line, MAX_LINE_LEN, reportFile)) {
        if (totalReports >= capacity) {
            capacity *= 2;
            reports = realloc(reports, capacity * sizeof(char*));
        }
        reports[totalReports++] = strdup(line);
    }

    double start = omp_get_wtime();

    // Parallel keyword count
    #pragma omp parallel for
    for (size_t i = 0; i < totalReports; i++) {
        for (int k = 0; k < totalKeywords; k++) {
            if (strstr(reports[i], keywords[k]) != NULL) {
                #pragma omp atomic
                keywordCount[k]++;
            }
        }
    }

    double end = omp_get_wtime();
    double time_taken = end - start;

    // Output
    FILE *out = fopen("../outputs/result_openmp.txt", "w");
    for (int i = 0; i < totalKeywords; i++) {
        fprintf(out, "%s: %d\n", keywords[i], keywordCount[i]);
    }
    fclose(out);

    FILE *perf = fopen("../outputs/performance.txt", "a");
    fprintf(perf, "OpenMP version time: %.6f seconds\t No. of Processors: 1\t No. of Threads: %d \n", time_taken, num_threads);
    fclose(perf);

    // Free memory
    for (size_t i = 0; i < totalReports; i++) {
        free(reports[i]);
    }
    free(reports);
    fclose(reportFile);
    fclose(keywordFile);

    printf("OpenMP done. Time: %.6f sec\n", time_taken);
    return 0;
}

