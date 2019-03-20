# Mini2048
Minimal, terminal-based clone of the popular game 2048 written in C.
Should work on any POSIX-compatible system. (For others, e.g. Windows, remove the termios stuff and the ANSI color sequences.)

![screenshot](https://raw.githubusercontent.com/hirschsn/mini2048/master/screenshot.png)

## Why yet another clone? Why "mini"?

2048 is an simple game. So should be it's implementation:

1. Single C file,
2. No dependencies but libc (and POSIX terminal interface + compatible terminal emulator),
3. Only ~200 sloc.

## Usage

Use w/a/s/d or 8/4/5/6 keys to move the blocks.

## License

See LICENSE (ISC).
