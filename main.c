#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>


void print_welcome_message() {
    printf("#======================================================#\n");
    printf("#-------------------- API FUZZER ----------------------#\n");
    printf("#======================================================#\n");
}


void print_usage() {
    printf("Usage: program [-h] [-v] [-f filename] [-n fuzzing_attempts]\n");
    printf("Options:\n");
    printf("  -h        Show this help message\n");
    printf("  -v        Enable verbose mode\n");
    printf("  -f        Specify input filename with list of endpoints\n");
    printf("  -n        Specify a number of fuzzing attempts\n");
}


extern int fuzz(const char *filename, int attempts, int verbose);


int main(int argc, char *argv[]) {
    int verbose = 0;
    char *filename = NULL;
    int attempts = 0;
    
    if (argc == 1) {
        print_usage();
        return 0;
    }

    int opt;
    while ((opt = getopt(argc, argv, "hvf:n:")) != -1) {
        switch (opt) {
            case 'h':
                print_usage();
                return 0;
            case 'v':
                verbose = 1;
                break;
            case 'f':
                filename = optarg;
                break;
            case 'n':
                attempts = atoi(optarg);
                break;
            case '?':
                print_usage();
                return 1;
            default:
                abort();
        }
    }
    print_welcome_message();
    printf("Running with: ");
    printf("Verbose mode: %s\n", verbose ? "ON" : "OFF");
    if (filename) {
        printf("Filename: %s\n", filename);
    }
    if (attempts) {
        printf("Number of fuzzing attempts: %d\n", attempts);
    }

    if (optind < argc) {
        printf("Non-option arguments: ");
        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }
        printf("\n");
    }


    fuzz(filename, attempts, verbose);

    return 0;
}