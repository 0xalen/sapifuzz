#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include "fuzzer.h"

// Structure to hold endpoint and method
typedef struct {
    const char *url;
    const char *method; // "GET" or "POST"
} Endpoint;

// Callback to handle API response
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    printf("Response: %.*s\n", (int)realsize, (char *)contents);
    return realsize;
}

// Generate random string of given length
char *generate_payload(size_t length) {
    char *payload = malloc(length + 1);
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789;'\0";
    for (size_t i = 0; i < length; i++) {
        payload[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    payload[length] = '\0';
    return payload;
}

// Build URL with query string for GET requests
char *build_get_url(const char *base_url, const char *payload) {
    size_t url_len = strlen(base_url) + strlen(payload) + 6; // "?data=" + payload + null
    char *full_url = malloc(url_len);
    snprintf(full_url, url_len, "%s?data=%s", base_url, payload);
    return full_url;
}

// Function to count lines in the file
int count_lines(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return -1;
    }
    
    int lines = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp)) {
        lines++;
    }
    fclose(fp);
    return lines;
}

// Function to load endpoints from file
Endpoint *load_endpoints(const char *filename, int *num_endpoints) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        *num_endpoints = 0;
        return NULL;
    }

    int lines = count_lines(filename);
    if (lines <= 0) {
        fclose(fp);
        *num_endpoints = 0;
        return NULL;
    }

    Endpoint *endpoints = malloc(lines * sizeof(Endpoint));
    if (!endpoints) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        *num_endpoints = 0;
        return NULL;
    }

    rewind(fp); 
    char buffer[1024];
    int index = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;

        char *url = strtok(buffer, ",");
        char *method = strtok(NULL, " ");

        if (url && method) {
            while (*url == ' ') url++;
            while (*method == ' ') method++;

            endpoints[index].url = strdup(url);
            endpoints[index].method = strdup(method);
            index++;
        }
    }

    fclose(fp);
    *num_endpoints = index;
    return endpoints;
}

void free_endpoints(Endpoint *endpoints, int num_endpoints) {
    for (int i = 0; i < num_endpoints; i++) {
        free(endpoints[i].url);
        free(endpoints[i].method);
    }
    free(endpoints);
}


int fuzz(const char *filename, int attempts, int verbose) {
    CURL *curl;
    CURLcode res;
    srand(time(NULL));

    
    int num_endpoints;
    Endpoint *endpoints = NULL;

    if (filename) {
        endpoints = load_endpoints(filename, &num_endpoints);
        if (!endpoints) {
            fprintf(stderr, "No valid endpoints loaded from %s, exiting\n", filename);
            return 1;
        }
    } else {
        printf("Filename must be provided");
        return 1;
    }

    int num_attempts = attempts > 0 ? attempts : 25;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Curl init failed\n");
        return 1;
    }

    for (int e = 0; e < num_endpoints; e++) {
        const char *url = endpoints[e].url;
        const char *method = endpoints[e].method;

        for (int i = 0; i < num_attempts; i++) {
            size_t len = (rand() % 1000) + 1;
            char *payload = generate_payload(len);

            if (strcmp(method, "POST") == 0) {
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                printf("Sending POST payload %d to %s: %s\n", i + 1, url, payload);
            } else if (strcmp(method, "GET") == 0) {
                char *full_url = build_get_url(url, payload);
                curl_easy_setopt(curl, CURLOPT_URL, full_url);
                curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
                printf("Sending GET payload %d to %s: %s\n", i + 1, full_url, payload);
                free(full_url);
            }

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Request failed for %s: %s\n", url, curl_easy_strerror(res));
            }

            free(payload);
        }
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free_endpoints(endpoints, num_endpoints);
    return 0;
}

