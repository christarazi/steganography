# steganography
A command-line C program that hides / reveals messages in Bitmap images (BMP)
using steganography. This program will work on any Linux machine.

**Disclaimer**: this was created for educational purposes. I do not take any
responsibility for any misuse of this program.

## Usage

### Dependencies

This program will compile on any Linux machine with:

 - gcc (gnu11 / c11 support)
 - make

### Build / Installation

Simply download / clone the repo then run:

```shell
$ cd /path/to/code/
$ make
```

The executable `steg` should be created.

To use `steg`:

```shell
# Encode the message below into the provided sample BMP `tree.bpm`
# This will output a file in the current directory like `fileXXXXXX`
$ ./steg -e "Hidden message" samples/tree.bmp

# Decode the message from above
$ ./steg -c 20 -d `fileXXXXXX`

# See more usage help
$ ./steg -h
```

## How does it work?

Bitmap images are very simple files. They contain some header information, then
raw RGB pixels. The RGB pixels can be thought of as:

```c
struct RGB {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};
```

As you can see, there is 1 byte for every channel. To perform steganography, we
can replace one of the channels with a byte from the message.

For example, if your message was "hello", then the 'h' will replace the first
RGB byte, 'e' will replace the second RGB byte, and so on.

In this program, I chose to overwrite the blue channel for no other reason than
it is the first channel in Bitmap images.

**Note**: there are 7 different types of Bitmap files. See this Wikipedia page:
https://en.wikipedia.org/wiki/BMP_file_format to read more about them.
This program only supports the `BITMAPV5HEADER` type and 24 bpp format (meaning
8 bits per channel [1 byte]) which seems to be the most popular type on the
Internet. The images in `samples/` are some examples of that.

## TODO

 - Add LSB (least significant bit) method instead of simply overwriting bytes
 - Add images within images
 - Add more support for different images

## Contribution

Please fork and submit a pull request. If you find a bug, submit an issue. I welcome feedback as well.
