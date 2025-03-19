#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "fuzzer.h"


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
    printf("  -e        Specify endpoint to fuzz. Works with -m\n");
    printf("  -m        Specify method from endpoint to fuzz. Works with -e\n");
    printf("  -n        Specify a number of fuzzing attempts\n");
    printf("Examples:\n");
    printf("\n");
    printf("Filename mode:\n");
    printf("    fuzzapi -f endpoints.txt -v -n 10");
    printf("\n");
    printf("Single endpoint mode:\n");
    printf("    fuzzapi -e http://example.com/api -m GET -v -n 5");
    printf("\n");
}


int main(int argc, char *argv[]) {
    int verbose = 0;
    char *filename = NULL;
    int attempts = 0;
    char *endpoint = NULL;
    char *method = NULL;
    
    if (argc == 1) {
        print_usage();
        return 0;
    }

    int opt;
    while ((opt = getopt(argc, argv, "hvf:e:m:n:")) != -1) {
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
            case 'e':
                endpoint = optarg;
                break;
            case 'm':
                method = optarg;
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
    printf("Session parameters: \n");
    printf("Verbose mode: %s\n", verbose ? "ON" : "OFF");
    if (filename) {
        printf("Filename: %s\n", filename);
    }
    if (endpoint && method) {
        printf("Target: %s (%s)\n", endpoint, method);
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

    printf("\nFuzzing...\n");
    int res;
    if (filename) {
        if (endpoint || method) {
            printf("Warning: -f cannot be used with -e or -m. Ignoring -e and -m\n");
        }
        res = fuzz_from_file(filename, attempts, verbose);
    } else if (endpoint && method) {
        res = fuzz_endpoint(endpoint, method, attempts, verbose);
    } else {
        printf("Error: Must provide either -f filename or both -e endpoint and -m method\n");
        print_usage();
        return 1;
    }
    
    if (res != 0) {
        return 1;
    }


    return 0;
}