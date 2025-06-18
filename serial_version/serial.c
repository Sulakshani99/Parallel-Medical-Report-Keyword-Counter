#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_KEYWORDS 100
#define MAX_LINE_LENGTH 1000
#define MAX_KEYWORD_LENGTH 50

int main() {
    // File pointers
    FILE *report_file;
    FILE *keyword_file;
    FILE *output_file;
    FILE *performance_file;

    // Arrays to store keywords and their counts
    char keywords[MAX_KEYWORDS][MAX_KEYWORD_LENGTH];
    int keyword_counts[MAX_KEYWORDS] = {0};
    int num_keywords = 0;

    char line[MAX_LINE_LENGTH];

    clock_t start_time = clock();

    // Open the medical reports file
    report_file = fopen("../data/medical_reports.txt", "r");
    if (report_file == NULL) {
        printf("Error: Cannot open medical_reports.txt\n");
        return 1;
    }

    // Open the keywords file
    keyword_file = fopen("../keywords.txt", "r");
    if (keyword_file == NULL) {
        printf("Error: Cannot open keywords.txt\n");
        return 1;
    }

    // Read all keywords from the file
    while (fgets(keywords[num_keywords], MAX_KEYWORD_LENGTH, keyword_file)) {
        keywords[num_keywords][strcspn(keywords[num_keywords], "\n")] = '\0';
        num_keywords++;
    }
    fclose(keyword_file);

    // Read each line from the medical reports file
    while (fgets(line, MAX_LINE_LENGTH, report_file)) {
        for (int i = 0; i < num_keywords; i++) {
            if (strstr(line, keywords[i]) != NULL) {
                keyword_counts[i]++;
            }
        }
    }
    fclose(report_file);
    
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Write the keyword counts to the output file
    output_file = fopen("../outputs/result_serial.txt", "w");
    if (output_file == NULL) {
        printf("Error: Cannot open result_serial.txt for writing\n");
        return 1;
    }
    for (int i = 0; i < num_keywords; i++) {
        fprintf(output_file, "%s: %d\n", keywords[i], keyword_counts[i]);
    }
    fclose(output_file);


    // Write performance info to file
    performance_file = fopen("../outputs/performance.txt", "a");
    if (performance_file != NULL) {
        fprintf(performance_file, "Serial version time: %.6f seconds\t No. of Processors: 1\t No. of Threads: 1\n", elapsed_time);
        fclose(performance_file);
    }

    printf("Done! Results saved to result_serial.txt\n");
    printf("Time taken: %.6f seconds\n", elapsed_time);

    return 0;
}

