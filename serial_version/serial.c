#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>  

#define MAX_KEYWORDS 100
#define MAX_LINE 1000

int main() {

    FILE *reportFile, *keywordFile;
    char reportLine[MAX_LINE];
    char keywords[MAX_KEYWORDS][50];
    int keywordCount[MAX_KEYWORDS] = {0};
    int totalKeywords = 0;

    // Start timer
    clock_t start = clock();

    // 1. Open the medical report file
    reportFile = fopen("../data/medical_reports.txt", "r");
    if (reportFile == NULL) {
        printf("Cannot open medical_reports.txt\n");
        return 1;
    }

    // 2. Open the keyword file
    keywordFile = fopen("../keywords.txt", "r");
    if (keywordFile == NULL) {
        printf("Cannot open keywords.txt\n");
        return 1;
    }

    // 3. Read all keywords and store them in an array
    while (fgets(keywords[totalKeywords], 50, keywordFile)) {
        // Remove newline (\n) at the end
        keywords[totalKeywords][strcspn(keywords[totalKeywords], "\n")] = 0;
        totalKeywords++;
    }

    // 4. Read each line (report) and check for keywords
    while (fgets(reportLine, MAX_LINE, reportFile)) {
        for (int i = 0; i < totalKeywords; i++) {
            // strstr checks if keyword exists in the line
            if (strstr(reportLine, keywords[i]) != NULL) {
                keywordCount[i]++;
            }
        }
    }

    // 5. Save the result to result_serial.txt
    FILE *outFile = fopen("../outputs/result_serial.txt", "w");
    for (int i = 0; i < totalKeywords; i++) {
        fprintf(outFile, "%s: %d\n", keywords[i], keywordCount[i]);
    }
    fclose(outFile);

    // Stop timer
    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    // 6. Save performance time
    FILE *perfFile = fopen("../outputs/performance.txt", "a");
    fprintf(perfFile, "Serial version time: %.6f seconds\t No. of Processors: 1\t No. of Treads: 1 \n", time_taken);
    fclose(perfFile);

    // Show user
    printf("Done! Results saved to result_serial.txt\n");
    printf("Time taken: %.6f seconds\n", time_taken);

    // Close files
    fclose(reportFile);
    fclose(keywordFile);

    return 0;
}

