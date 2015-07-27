# README #

This project provides a remote keyboard implementation, similar to [Unified Remote](https://www.unifiedremote.com/). The main difference is that it is between a Linux client and a Windows Host. All communication happens over TCPIP.

The Linux client's keyboard will be used to control the Windows computer almost seamlessly. It is even possible to play video games using this program.

The Server uses the Windows virtual keyboard interface to emulate key presses and releases.

Flags to pass:

* `-p <number>`: specify which TCPIP port to use.
* `--port <number>`: same as above
* `-x`: required flag if the client specifies it, otherwise this program will receive garbage. This allows almost all the keyboard keys to be used natively (for example, Ctrl, Alt, Shift, Home, End, etc, and in chords).
* `--xtended-features`: same as above