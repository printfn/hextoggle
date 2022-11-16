# hextoggle

## Build

To build and install `hextoggle`, you can run:

```sh
git clone https://github.com/printfn/hextoggle
cd hextoggle
sudo make install
```

You can optionally override the install location by setting `PREFIX`.
The default prefix is `/usr/local/`.

```sh
sudo make install PREFIX=.
```

## Usage

```
Usage: hextoggle [file]            # toggle file in-place
       hextoggle [input] [output]  # read 'input', write to 'output'
       hextoggle -                 # read from stdin/write to stdout

Flags:
       -n        --dry-run         # discard results
       -d        --decode          # force decode (i.e. hex -> binary)
       -e        --encode          # force encode (i.e. binary -> hex)
       -h        --help            # show this usage information

Return codes:
  0   success
  1   invalid arguments
  2   failed to open input files
  3   failed to clean up files
  4   invalid input
  5   internal assertion failed
```

## License

This project is available under the GPL 3.0 or any later version.
