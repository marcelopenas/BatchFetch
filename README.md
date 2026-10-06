# BatchFetch <img src="assets/vid/demo.gif" align="right" width="150" alt="BatchFetch demo">

> **BatchFetch** is a parallel, batched command-line downloader for fetching
> multiple URLs efficiently.

![Language](https://img.shields.io/badge/language-C11-blue)
![Build](https://img.shields.io/badge/build-Make-green)

## Overview

**BatchFetch** is built for downloading *many URLs at once*. It divides the
input into manageable **batches** and launches multiple downloads in parallel
within each batch.

Instead of starting an unbounded number of downloads, BatchFetch:

1. Reads and deduplicates the URL list.
2. Creates a batch containing up to `-N` URLs.
3. Starts one child process per URL in that batch.
4. Waits for the entire batch to finish.
5. Reports each result and starts the next batch.

This keeps parallelism controlled while allowing independent downloads to run
concurrently.

> [!NOTE]
> A more efficient downloader could keep the maximum number of processes busy:
> whenever one download completed, it could immediately start another URL.
> BatchFetch intentionally does **not** use that worker-pool approach. It waits
> for every download in the current batch to finish before starting the next
> batch, which can create idle time when downloads have different durations.
> This simpler batch-based design is deliberate because BatchFetch is an
> educational and testing project focused on demonstrating process creation,
> parallelism, and controlled batching rather than maximizing throughput.

It supports:

- Downloading a single URL.
- Downloading multiple URLs from a text file.
- Removing duplicate URLs from input files.
- Parallel downloads using multiple child processes.
- Batched scheduling to limit the number of active downloads.
- Automatic output filenames generated from URLs.
- Interactive `Ctrl+C` handling.
- Strict C11 compilation with warnings enabled.

> [!IMPORTANT]
> This project is intended for educational and personal use. Always respect
> website terms of service, robots policies, copyright, and applicable laws.

## Requirements

- A POSIX-compatible operating system, such as Linux or macOS
- `gcc` or another C11-compatible compiler
- `make`
- The libcurl development package

### Installing dependencies on Debian or Ubuntu

```bash
sudo apt update
sudo apt install build-essential libcurl4-openssl-dev
```

## Building

Clone the repository and enter its directory:

```bash
git clone <repository-url>
cd BatchFetch
```

Build the program with:

```bash
make
```

The executable will be created as:

```text
./bin/batchfetch
```

To remove the compiled executable:

```bash
make clean
```

The build uses:

| Setting | Value |
| --- | --- |
| C standard | C11 |
| Compiler warnings | `-Wall -Wextra -Wpedantic` |
| Optimization/debugging | `-Og -g` |
| Library | `libcurl` |
| Default parallelism | 4 processes |
| Maximum parallelism | 64 processes |

## Usage

### Download one URL

```bash
./bin/batchfetch URL
```

Example:

```bash
./bin/batchfetch https://example.com/files/report.pdf
```

The output filename is generated automatically. For example:

```text
https://example.com/files/report.pdf
```

becomes approximately:

```text
example_com_files_report.pdf
```

### Download multiple URLs from a file

Create a file containing one URL per line:

```text
https://example.com/files/one.pdf
https://example.com/files/two.zip
https://example.com/files/one.pdf
```

Then run:

```bash
./bin/batchfetch -f urls.txt
```

BatchFetch processes the file in parallel batches. Empty lines are ignored, and
duplicate URLs are downloaded only once.

### Configure parallelism and batch size

Use `-N` to set the number of URLs downloaded concurrently in each batch:

```bash
./bin/batchfetch -f urls.txt -N 8
```

The default is **4** parallel processes per batch. Values above **64** are
capped automatically.

For example, with 20 URLs and `-N 4`, BatchFetch runs:

```text
Batch 1: URLs 1-4    (4 parallel downloads)
Batch 2: URLs 5-8    (4 parallel downloads)
Batch 3: URLs 9-12   (4 parallel downloads)
Batch 4: URLs 13-16  (4 parallel downloads)
Batch 5: URLs 17-20  (4 parallel downloads)
```

> [!TIP]
> Increase `-N` for faster throughput when the network and remote server can
> handle more simultaneous requests. A smaller value reduces resource usage and
> network pressure.

## Command-line options

| Option | Description | Default |
| --- | --- | --- |
| `URL` | Download one URL | — |
| `-f FILE` | Read URLs from `FILE` | — |
| `-N COUNT` | Set the number of parallel downloads per batch | `4` |

### Examples

```bash
# Single download
./bin/batchfetch https://example.com/image.jpg

# Download multiple URLs in parallel batches of four
./bin/batchfetch -f examples/list.txt

# Download using eight processes
./bin/batchfetch -f examples/list.txt -N 8

# Options may appear in either order
./bin/batchfetch -N 2 -f examples/list.txt
```

> [!WARNING]
> The program writes files into the current working directory. Check generated
> filenames before downloading content from untrusted or unfamiliar URLs.

## Interrupting downloads

Press `Ctrl+C` while a batch is running to pause the child processes. The
program asks whether you want to exit:

```text
Are you sure you want to exit? (y/n):
```

- Enter `y` or `Y` to terminate the active downloads.
- Enter any other response to resume them.

## Project structure

```text
.
├── src/
│   ├── arguments.c       # Command-line argument parsing
│   ├── downloader.c      # libcurl download implementation
│   ├── main.c            # Application orchestration
│   ├── signals.c         # SIGINT and child-process management
│   ├── url_list.c        # URL file loading and deduplication
│   └── url_utils.c       # URL-to-filename conversion
├── include/
│   ├── arguments.h
│   ├── config.h           # Shared configuration constants
│   ├── downloader.h
│   ├── signals.h
│   ├── url_list.h
│   └── url_utils.h
├── build/                # Generated object and dependency files
├── bin/                  # Generated executable
├── examples/list.txt     # Example URL list
├── Makefile
└── README.md
```

Each public module uses a header file with `#pragma once`, while implementation
details remain private to its corresponding `.c` file.

## Error handling

The program reports errors for common failures, including:

- Missing command-line arguments.
- Missing `-f` or `-N` values.
- Invalid process counts.
- Input files that cannot be opened.
- Memory allocation failures.
- Failed HTTP requests.
- Output files that cannot be written.

For a failed download, libcurl's human-readable error description is displayed.

## Development

Build with the same strict settings used by the project:

```bash
make clean
make
```

Useful checks:

```bash
# Check the executable exists
test -x ./bin/batchfetch

# Check invalid usage
./bin/batchfetch

# Check invalid process-count validation
./bin/batchfetch -f examples/list.txt -N invalid
```
