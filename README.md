# Simple API Fuzzer

[Repository](https://github.com/0xalen/sapifuzz)

## Description
SAPIFuzz is a command-line tool designed to fuzz API endpoints. It supports fuzzing multiple endpoints from a file or a single endpoint specified via command-line arguments. Built with libcurl, it generates random payloads and sends them as GET or POST requests, making it useful for testing API robustness and security.

Key features:
- Fuzz endpoints from a file (e.g., `endpoints.txt`) with `URL, METHOD` format.
- Fuzz a single endpoint with a specified method.
- Configurable number of fuzzing attempts.
- Verbose mode for detailed output.
- Graceful handling of mixed inputs with warnings.


## Author
    Franco Alejandro Trinidad <0xalen@disroot.org> 

## Installation

### Prerequisites

- GCC or another C compiler
- libcurl development libraries (e.g., `libcurl4-openssl-dev` on Debian/Ubuntu)
- Make

### Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/0xalen/sapifuzz.git
   cd sapifuzz
   ```

2. Build the tool:
    ```bash
    make
    ```

3. The executable fuzzapi will be created in the current directory.

## Usage

Run SAPIFuzz with the following options:
    ```bash
    ./fuzzapi [-h] [-v] [-f filename] [-n fuzzing_attempts]
    ./fuzzapi [-h] [-v] [-e endpoint] [-m method] [-n fuzzing_attempts]
    ```

### Options

    -h: Show help message and exit.

    -v: Enable verbose mode (prints payloads and request details).

    -f <filename>: Specify a file containing endpoints (format: URL,METHOD per line).

    -e <endpoint>: Specify a single endpoint URL (requires -m).

    -m <method>: Specify the HTTP method for the endpoint (requires -e; supports GET or POST).

    -n <attempts>: Number of fuzzing attempts (default: 25 if not specified).

### Notes
If -f is provided with -e or -m, a warning is issued, and -e/-m are ignored in favor of file-based fuzzing.

Without -f, both -e and -m must be provided together, or the tool will exit with an error.

## Examples

1. Fuzz endpoints from a file with verbose output:
    ```bash
    ./fuzzapi -f endpoints.txt -v -n 10
    ```

File endpoints.txt:
    ```
    http://example.com/api/endpoint1,POST
    http://example.com/api/endpoint2,GET
    ```

2. Fuzz a single endpoint:
    ```bash
    ./fuzzapi -e "http://example.com/api" -m "GET" -v -n 5
    ```

## Building and Running

### Requirements

- A C compiler (e.g., gcc)
- libcurl (install via sudo apt install libcurl4-openssl-dev on Debian/Ubuntu or equivalent).

### Makefile

The included Makefile compiles main.c and fuzzer.c into fuzzapi:

- Build: make
- Clean: make clean


## Project Structure

- main.c: Parses command-line arguments and invokes fuzzing functions.
- fuzzer.c: Core fuzzing logic, including endpoint loading and curl requests.
- fuzzer.h: Public interface for fuzzing functions.
- Makefile: Build configuration.

## Contributing
Feel free to submit issues or pull requests at https://github.com/0xalen/sapifuzz. Contributions to enhance functionality (e.g., more HTTP methods, payload customization) are welcome!



