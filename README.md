# Multimedia Data Processing

[![build](https://github.com/davide-lugli/MultimediaDataProcessing/actions/workflows/build.yml/badge.svg)](https://github.com/davide-lugli/MultimediaDataProcessing/actions/workflows/build.yml)

C++ programs I wrote for the Multimedia Data Processing course at the University of Modena and Reggio Emilia (UNIMORE) in 2024. The course is about how images, audio and other data are actually stored: binary and bit-level I/O, entropy coding, and reading and writing real file formats straight from their specification. Everything here uses only the standard library.

Most of these started as timed lab exercises, so each folder is a self-contained program that carries its own copy of the small helpers it needs. The bit reader/writer, for example, shows up in several places and gets a little better each time.

## What's in here

| Folder | Description |
| --- | --- |
| [qoa](qoa) | Decoder for the [Quite OK Audio](https://qoaformat.org) format. Reads a `.qoa` file and writes a 16-bit PCM `.wav`. |
| [huffdiff](huffdiff) | Lossless compressor for grayscale images. Each pixel is predicted from a neighbour and the residuals are coded with canonical Huffman codes. |
| [adam7](adam7) | Splits a PGM image into the seven interlaced levels of the Adam7 scheme (the one PNG uses) and rebuilds the progressive previews from them. |
| [torrent_dump](torrent_dump) | Bencode parser that prints the structure of a `.torrent` file. |
| [pam](pam) | Reader and writer for [PAM](https://netpbm.sourceforge.net/doc/pam.html) images, used to generate a gradient and to flip images vertically and horizontally. |
| [inheritance](inheritance) | ASCII-art canvas with Bresenham lines and circles, drawn through a small class hierarchy with a virtual `draw()`. |
| [bit_rw](bit_rw) | Packs integers into 11-bit two's complement fields and reads them back. |
| [binary_rw](binary_rw) | Converts integers between text and raw 32-bit binary. |
| [frequencies](frequencies) | Byte histogram and Shannon entropy of a file. |
| [sort_int](sort_int) | Reads integers from a file and sorts them. Not interesting by itself, but see below. |

## Notes on some of the programs

### qoa

```
qoa <input.qoa> <output.wav>
```

QOA is a simple lossy audio codec. The file is a sequence of frames; each frame stores, per channel, the state of an LMS predictor (the last four samples and four weights), followed by slices of 20 samples. Every slice has a 4-bit scale factor and twenty 3-bit quantized residuals. Decoding a sample means predicting it from the history, dequantizing and scaling the residual, adding the two, clamping to 16 bits, and then updating the predictor weights and history before moving on to the next one.

The [QOA website](https://qoaformat.org) has the specification and sample files to try it on (none are included here).

### huffdiff

```
huffdiff c <input.pam> <output.hd>
huffdiff d <input.hd> <output.pam>
```

Works on 8-bit grayscale PAM images. Every pixel is replaced by its difference from the pixel to its left (the first pixel of each row uses the one above it instead). On a natural image this turns a fairly flat distribution of values into one sharply peaked around zero, which Huffman coding handles well. Residuals go from -255 to 255, so they are stored as 9-bit symbols.

The code is built from the actual symbol frequencies and then converted to canonical form, so the file only needs to store each symbol's code length and the decoder can rebuild the exact same codes. The layout, with every field written most significant bit first:

| Field | Size |
| --- | --- |
| Magic number `HUFFDIFF` | 8 bytes |
| Width, height | 32 bits each |
| Number of table entries | 9 bits |
| Table entries: symbol, code length | 9 + 5 bits each |
| One Huffman code per pixel | variable |

On the sample `frog.pam` (375x266) it goes from 99,819 bytes to 53,071, about 4.3 bits per pixel.

`huffman.h` also has a plain byte-oriented Huffman compressor from an earlier exercise, in two variants: one that stores the codes explicitly (`HUFFMAN1`) and one that uses canonical codes (`HUFFMAN2`).

### adam7

```
adam7 c <input.pgm> <output.mlt>
adam7 d <input.mlt> <prefix>
```

Encoding reorders the pixels of an 8-bit grayscale [PGM](https://netpbm.sourceforge.net/doc/pgm.html) image following the 8x8 Adam7 pattern: first all the level 1 pixels (one per 8x8 block), then level 2, and so on up to level 7, which is every other row. The output is the magic number `MULTIRES`, width and height as 32-bit little-endian integers, and then the pixel bytes level by level.

Decoding writes seven images, `<prefix>_1.pgm` to `<prefix>_7.pgm`, showing what a viewer would have after receiving each level. Every pixel that hasn't arrived yet takes the value of the nearest received pixel above and to the left, so the image goes from 8x8 blocks to full resolution, and the last one has exactly the original pixels.

### torrent_dump

```
torrent_dump <file.torrent>
```

Recursive descent parser for [bencode](https://www.bittorrent.org/beps/bep_0003.html), the format `.torrent` files are written in (integers, byte strings, lists and dictionaries). It prints the tree with indentation, replaces non-printable bytes with `.`, and shows the `pieces` field, which is the concatenation of the SHA-1 hashes of every piece, as hex with one hash per line.

### sort_int

This one is about the path rather than the result. The `versions` folder has the same program at each step of the first weeks of the course:

1. C, with an array grown by `realloc` and sorted with `qsort`
2. C, with the array wrapped in a struct and constructor/destructor-style functions
3. C++, where the struct gets member functions, a constructor and a destructor
4. Copy constructor and copy assignment, with notes on what goes wrong with the default shallow copy
5. Move constructor and move assignment
6. Copy-and-swap, so one assignment operator covers both copy and move
7. Namespaces, argument-dependent lookup, `const` overloads and member initializer lists
8. The vector becomes a class template, split into a header and a source file
9. All of the above replaced by `std::vector` and `std::sort`

`main.cpp` is the final step: reading a file with stream iterators, lambdas, function objects, and a detour into the most vexing parse. The comments in these files are my notes from the lectures, so they explain things that would normally go without saying.

## Building

You need CMake 3.20 or newer and a compiler that supports C++20.

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```

The executables end up in `build/bin`, or `build/bin/Release` with the Visual Studio generator. Visual Studio can also open the folder directly as a CMake project. The intermediate versions in `*/versions` are built too; pass `-DMDP_BUILD_VERSIONS=OFF` to skip them.

The tests run the encoders and decoders on the sample files and check that the round trip gives back the original data. CI builds and tests everything on Windows with MSVC and on Ubuntu with GCC.

## Trying it out

From the repository root, after building:

```sh
build/bin/huffdiff c huffdiff/frog.pam frog.hd
build/bin/huffdiff d frog.hd frog_decoded.pam

build/bin/adam7 c adam7/rana.pgm rana.mlt
build/bin/adam7 d rana.mlt rana

build/bin/torrent_dump torrent_dump/sample.torrent
```

`pam` takes no arguments and runs its three exercises on `frog.pam` and `laptop.pam`, so run it from inside the `pam` folder. To look at the resulting images you need a viewer that reads PAM and PGM, such as XnView.

## Limitations

These programs follow the assignment specs and were not meant to be general-purpose tools, so error handling is minimal. In particular:

- `qoa` only handles stereo files, and always writes a 44.1 kHz WAV header.
- `pam`, `huffdiff` and `adam7` only handle 8-bit samples (`MAXVAL` up to 255).
