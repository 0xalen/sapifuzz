#ifndef FUZZER_H
#define FUZZER_H

int fuzz_from_file(const char *filename, int attempts, int verbose);
int fuzz_endpoint(const char *endpoint, const char *method, int attempts, int verbose);

#endif