# PixOMP
`PixOMP` is a C++23 command-line image processing tool and small processing library for binary Netpbm images. The CLI can run contrast correction, resize, or both operations while comparing different execution policies.

## Features

- reads and writes binary `P6` PPM and `P5` PGM images with `max_value = 255`;
- contrast stretching based on the image brightness histogram;
- resize with nearest, bilinear, and bicubic interpolation;
- RGB and grayscale image processing;
- execution modes for single-thread, OpenMP `parallel for`, and manual OpenMP work distribution;
- configurable thread count, scheduling policy, and chunk size;
- benchmark mode with warmup runs and averaged processing time.

## Requirements

- CMake 4.2.1 or newer;
- a C++23 compiler;
- OpenMP;
- AVX2 support for `src/operation/contrast_op.cpp`, which is compiled with `-mavx2`.

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

The executable is produced at:

```bash
./build/mainexecutable
```

If Ninja is not installed, use the default CMake generator:

```bash
cmake -S . -B build
cmake --build build
```

## Usage

Contrast only:

```bash
./build/mainexecutable \
  --in input.ppm \
  --out output.ppm \
  --operation contrast \
  --coef 0.01 \
  --benchmark 0
```

Resize only:

```bash
./build/mainexecutable \
  --in input.ppm \
  --out resized.ppm \
  --operation resize \
  --resize_width 1280 \
  --resize_height 720 \
  --resize_policy bilinear \
  --benchmark 0
```

Run contrast first, then resize:

```bash
./build/mainexecutable \
  --in input.ppm \
  --out processed.ppm \
  --operation both \
  --coef 0.02 \
  --resize_width 1280 \
  --resize_height 720 \
  --resize_policy bicubic \
  --benchmark 0
```

OpenMP run with 8 threads:

```bash
./build/mainexecutable \
  --in input.ppm \
  --out output.ppm \
  --operation both \
  --coef 0.01 \
  --resize_width 1280 \
  --resize_height 720 \
  --realization 2 \
  --num_threads 8 \
  --schedule static \
  --benchmark 0
```

Manual OpenMP distribution with dynamic scheduling:

```bash
./build/mainexecutable \
  --in input.ppm \
  --out output.ppm \
  --operation resize \
  --resize_width 800 \
  --resize_height 600 \
  --resize_policy nearest \
  --realization 3 \
  --num_threads 8 \
  --chunk_size 1024 \
  --schedule dynamic \
  --benchmark 0
```

## CLI Options

| Option | Short | Default | Description |
| --- | --- | --- | --- |
| `--in` | `-i` | empty | Input image path. Required. |
| `--out` | `-o` | empty | Output image path. Required. |
| `--operation` | | `contrast` | Processing mode: `contrast`, `resize`, or `both`. In `both` mode, contrast runs before resize. |
| `--coef` | `-c` | `0.0` | Contrast cutoff ratio removed from both ends of the brightness histogram. Valid range: `[0.0, 0.5)`. Used by `contrast` and `both`. |
| `--resize_width` | | `0` | Output width for resize. Required for `resize` and `both`. |
| `--resize_height` | | `0` | Output height for resize. Required for `resize` and `both`. |
| `--resize_policy` | | `bilinear` | Resize interpolation: `nearest`, `bilinear`, or `bicubic`. |
| `--realization` | `-r` | `1` | Execution mode: `1` = single-thread, `2` = OpenMP `parallel for`, `3` = manual OpenMP distribution. |
| `--num_threads` | `-t` | `0` | Thread count. `0` keeps the OpenMP default. |
| `--chunk_size` | | `0` | Scheduler chunk size. In manual mode, `0` chooses the chunk size automatically. |
| `--schedule` | `-s` | `static` | Scheduling policy: `static` or `dynamic`. |
| `--benchmark` | `-b` | `1` | `1` enables benchmark mode with 5 warmup runs and 50 measured runs. `0` processes once. |

## Supported Image Formats

Only binary Netpbm files are supported:

- `P6` RGB PPM;
- `P5` grayscale PGM;
- channel maximum value must be `255`.

Text Netpbm formats `P2`/`P3`, files with another `max_value`, PNG, JPEG, and other common image formats are not supported directly.

## Examples

Single-thread contrast correction:

```bash
./build/mainexecutable -i image.ppm -o image_out.ppm -c 0.02 -r 1 -b 0
```

Benchmark bicubic resize with OpenMP:

```bash
./build/mainexecutable \
  -i image.ppm \
  -o image_resized.ppm \
  --operation resize \
  --resize_width 1920 \
  --resize_height 1080 \
  --resize_policy bicubic \
  -r 2 \
  -t 8 \
  -s static \
  -b 1
```

With `--benchmark 1`, the program prints the average processing time across 50 measured runs and the total measured time. With `--benchmark 0`, it prints the processing time for one run.
