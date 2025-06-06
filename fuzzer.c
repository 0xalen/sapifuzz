#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include "fuzzer.h"

// Structure to hold endpoint and method
typedef struct {
    char *url;
    char *method; // "GET" or "POST"
} Endpoint;


static Endpoint *load_endpoints(const char *filename, int *num_endpoints);
static void free_endpoints(Endpoint *endpoints, int num_endpoints);
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
static char *generate_payload(size_t length);
static char *build_get_url(const char *base_url, const char *payload);
static int fuzz(CURL *curl, const char *url, const char *method, int attempts, int verbose);


// Callback to handle API response
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    printf("Response: %.*s\n", (int)realsize, (char *)contents);
    return realsize;
}

// Generate random string of given length
static char *generate_payload(size_t length) {
    char *payload = malloc(length + 1);
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789;'\0";
    for (size_t i = 0; i < length; i++) {
        payload[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    payload[length] = '\0';
    return payload;
}

// Build URL with query string for GET requests
static char *build_get_url(const char *base_url, const char *payload) {
    size_t url_len = strlen(base_url) + strlen(payload) + 1; //  payload + null terminator '\0'
    char *full_url = malloc(url_len);
    snprintf(full_url, url_len, "%s%s", base_url, payload);
    return full_url;
}


// Function to load endpoints from file
static Endpoint *load_endpoints(const char *filename, int *num_endpoints) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        *num_endpoints = 0;
        return NULL;
    }

    Endpoint *endpoints = NULL;
    int capacity = 0;  // Current allocated size of endpoints array
    int index = 0;     // Number of endpoints actually used
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;

        char *url = strtok(buffer, ",");
        char *method = strtok(NULL, " ");

        if (url && method) {
            if (index >= capacity) {
                capacity = capacity == 0 ? 4 : capacity * 2;
                Endpoint *temp = realloc(endpoints, capacity * sizeof(Endpoint));
                if (!temp) {
                    fprintf(stderr, "Memory reallocation failed\n");
                    free_endpoints(endpoints, index);
                    fclose(fp);
                    *num_endpoints = 0;
                    return NULL;
                }
                endpoints = temp;
            }

            while (*url == ' ') url++;
            while (*method == ' ') method++;

            endpoints[index].url = strdup(url);
            endpoints[index].method = strdup(method);

            if (!endpoints[index].url || !endpoints[index].method) {
                fprintf(stderr, "Failed to load endpoints into memory\n");
                free_endpoints(endpoints, index + 1);
                fclose(fp);
                *num_endpoints = 0;
                return NULL;
            }

            index++;
        }
    }

    fclose(fp);
    *num_endpoints = index;

    if (index < capacity) {
        Endpoint *temp = realloc(endpoints, index * sizeof(Endpoint));
        if (temp) endpoints = temp;
    }

    return endpoints;
}

static void free_endpoints(Endpoint *endpoints, int num_endpoints) {
    for (int i = 0; i < num_endpoints; i++) {
        free(endpoints[i].url);
        free(endpoints[i].method);
    }
    free(endpoints);
}


static int fuzz(CURL *curl, const char *url, const char *method, int attempts, int verbose) {
    CURLcode res;
    int num_attempts = attempts > 0 ? attempts : 25;

    for (int i = 0; i < num_attempts; i++) {
        size_t len = (rand() % 1000) + 1;
        char *payload = generate_payload(len);
        if (!payload) {
            fprintf(stderr, "Failed to generate payload\n");
            return 1;
        }

        if (strcmp(method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            if (verbose) {
                printf("Sending POST payload %d to %s: %s\n", i + 1, url, payload);
            }
        } else if (strcmp(method, "GET") == 0) {
            char *full_url = build_get_url(url, payload);
            if (!full_url) {
                fprintf(stderr, "Failed to build GET URL\n");
                free(payload);
                return 1;
            }
            curl_easy_setopt(curl, CURLOPT_URL, full_url);
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            if (verbose) {
                printf("Sending GET payload %d to %s: %s\n", i + 1, full_url, payload);
            }
            free(full_url);
        } else {
            fprintf(stderr, "Unsupported method: %s\n", method);
            free(payload);
            return 1;
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Request failed for %s: %s\n", url, curl_easy_strerror(res));
        }

        free(payload);
    }

    return 0;
}


int fuzz_from_file(const char *filename, int attempts, int verbose) {
    CURL *curl;
    srand(time(NULL));

    int num_endpoints;
    Endpoint *endpoints = load_endpoints(filename, &num_endpoints);
    if (!endpoints) {
        fprintf(stderr, "No valid endpoints loaded from %s, exiting\n", filename);
        return 1;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Curl init failed\n");
        free_endpoints(endpoints, num_endpoints);
        return 1;
    }

    int res = 0;
    for (int e = 0; e < num_endpoints && res == 0; e++) {
        res = fuzz(curl, endpoints[e].url, endpoints[e].method, attempts, verbose);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free_endpoints(endpoints, num_endpoints);
    return res;
}

int fuzz_endpoint(const char *endpoint, const char *method, int attempts, int verbose) {
    CURL *curl;
    srand(time(NULL));

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Curl init failed\n");
        return 1;
    }

    int res = fuzz(curl, endpoint, method, attempts, verbose);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return res;
}