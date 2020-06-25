# wad2bin

Converts installable Wii WAD packages to backup WAD packages (*.bin files) using console-specific keydata. These files can be stored on a SD card and used to launch channels via System Menu 4.0+, or used with games that save/read data in this format.

Usage:
--------------

```
Usage: %s <keys file> <device.cert> <input WAD> <output dir> [parent title ID]

Paths must not exceed 1023 characters. Relative paths are supported.
The required directory tree for the *.bin file(s) will be created at the output directory.
You can set your SD card root directory as the output directory.
Parent title ID is only required if the input WAD is a DLC. A 16 character long hex string is expected.
```

Building instructions:
--------------

* **Windows** (should you really bother? 32-bit binaries are provided):
    1. A collection of Unix/Linux utilities such as `make`, `mkdir`, `rm`, `ar` and `cp`, as well as a `gnu11` standard compliant C compiler, are needed to build this project. There are a few ways to achieve this under Windows:
        * Install [TDM-GCC](https://jmeubank.github.io/tdm-gcc/download) and [devkitPro + MSYS2](https://devkitpro.org/wiki/Getting_Started#Windows), or...
        * Install [MinGW + MSYS](http://www.mingw.org/wiki/Getting_Started), or...
        * (Untested) Under Windows 10:
            * Install the [Windows Subsystem for Linux (WSL)](https://docs.microsoft.com/en-us/windows/wsl/install-win10) distribution of your preference (I'll assume you chose Ubuntu).
            * Install the `gcc` package by using the `sudo apt update` and `sudo apt install gcc` commands.
            * Modify the Makefiles to use `wsl.exe` before each Unix command call.
        * Regardless of the option you end up choosing, please make sure your `PATH` environment variable has been properly updated!
    2. Create a copy of `config.mk.template` in the same directory and rename it to `config.mk`.
    3. Build using the `make` command.

* **Unix-like OS (Linux / MacOS)**:
    1. Install your preferred C compiler compatible with the `gnu11` standard using your OS's package manager (e.g. `apt`, `pacman`, `brew`, etc.).
    2. Create a copy of `config.mk.template` in the same directory and rename it to `config.mk`. Open it with a text editor.
    3. Update the `CC` variable to make it point to your installed C compiler (leaving `gcc` is usually fine), and wipe the value from the `EXE_EXT` variable.
    4. Build using the `make` command.

Guidelines:
--------------

* Console-specific data is required to perform the conversion. Dump it from the target console using [xyzzy-mod](https://github.com/DarkMatterCore/xyzzy-mod).
    * The program expects two different files with console specific data: a text file with keydata (check `keys.txt.template` for the actual format) and `device.cert`. Both files are generated by xyzzy-mod.
* Both ticket and TMD for each converted WAD package must be installed on the target Wii console in order for this to work.
    * For this matter, the program generates a bogus WAD package at the provided output directory, using the `<title_id>_bogus.wad` naming convention. It can be used with regular WAD Managers to install both ticket and TMD if needed.
    * Bogus WAD packages don't have any encrypted content data, thus they're expected to return errors like `-1022` during installation with regular WAD Managers - this is expected behaviour, so don't panic.
    * If you can't get your converted WAD package to work, try "uninstalling" the bogus WAD before installing it. It fixes the problem most of the time.
* If the WAD ticket wasn't issued for the target console, or if the WAD isn't legit (e.g. homebrew WAD), the IOS used by the System Menu must be patched to enable the [signing bug](https://wiibrew.org/wiki/Signing_bug) on it.
    * Regular Wii consoles: use [Simple IOS Patcher](https://gbatemp.net/download/simple-ios-patcher.25749) to install a patched System Menu IOS (IOS60 for 4.0 and 4.1, IOS70 for 4.2 and IOS80 for 4.3).
    * Wii U consoles (vWii): use [Patched IOS80 Installer for vWii](https://gbatemp.net/threads/patched-ios80-installer-for-vwii-allows-sd-menu-custom-channels.344882) to install a patched IOS80.
* Channel WADs get converted to `content.bin` files, while DLC WADs get converted to `<index>.bin` files.
* If a DLC WAD is provided, it doesn't matter if it's an uncomplete WAD with missing contents, a WAD with a tampered TMD that only references the packaged contents or a full WAD with all contents: all cases are supported by wad2bin. There's no need to provide any content indexes.

Supported DLCs:
--------------

Not all DLCs can be converted to `<index>.bin` files - this is because not all games with DLCs are capable of loading DLC data stored on the SD card. For this purpose, wad2bin holds a hardcoded DLC title ID list, in order to avoid converting a DLC WAD that simply won't work from the SD card.

The title ID from the parent game must be provided as an additional command line argument, because some games can "import" DLC data that belongs to other games. Only the following DLCs are supported:

* Rock Band 2 (`00010000-535A41xx`) (`SZAx`):
    * `00010005-735A41xx` (`sZAx`).
    * `00010005-735A42xx` (`sZBx`).
    * `00010005-735A43xx` (`sZCx`).
    * `00010005-735A44xx` (`sZDx`).
    * `00010005-735A45xx` (`sZEx`).
    * `00010005-735A46xx` (`sZFx`).

* The Beatles Rock Band (`00010000-52394Axx`) (`R9Jx`):
    * `00010005-72394Axx` (`r9Jx`).

* Rock Band 3 (`00010000-535A42xx`) (`SZBx`):
    * `00010005-735A4Axx` (`sZJx`).
    * `00010005-735A4Bxx` (`sZKx`).
    * `00010005-735A4Cxx` (`sZLx`).
    * `00010005-735A4Dxx` (`sZMx`).

* Guitar Hero: World Tour (`00010000-535841xx`) (`SXAx`):
    * `00010005-735841xx` (`sXAx`).
    * `00010005-73594Fxx` (`sYOx`).

* Guitar Hero 5 (`00010000-535845xx`) (`SXEx`):
    * `00010005-735845xx` (`sXEx`).
    * `00010005-735846xx` (`sXFx`).
    * `00010005-735847xx` (`sXGx`).
    * `00010005-735848xx` (`sXHx`).

* Guitar Hero: Warriors of Rock (`00010000-535849xx`) (`SXIx`):
    * `00010005-735849xx` (`sXIx`).

* Just Dance 2 (`00010000-534432xx`) (`SD2x`):
    * `00010005-734432xx` (`sD2x`).

* Just Dance 3 (`00010000-534A44xx`) (`SJDx`):
    * `00010005-734A44xx` (`sJDx`).

* Just Dance 4 (`00010000-534A58xx`) (`SJXx`):
    * `00010005-734A58xx` (`sJXx`).

* Just Dance 2014 (`00010000-534A4Fxx`) (`SJOx`):
    * `00010005-734A4Fxx` (`sJOx`).

* Just Dance 2015 (`00010000-534533xx`) (`SE3x`):
    * `00010005-734533xx` (`sE3x`).

Any DLCs not appearing on this list will return an error if used as the input WAD package for the program. If you come across a DLC that can be loaded from the SD card and doesn't appear on this list, please contact me or open an issue and I'll gladly add it.

Differences between `content.bin` files and `<index>.bin` files:
--------------

* `content.bin` files are used to store data from `00010001` (downloadable channels) and `00010004` (disc-based channels) titles, and get saved to `sd:/private/wii/title/<ascii_lower_tid>/content.bin`. Whilst `<index>.bin` files are used to store data from `00010005` (DLC) titles, and get saved to `sd:/private/wii/data/<ascii_lower_tid>/<index>.bin` - where `<index>` represents a specific content index from its TMD (000 - 511).
* Both `content.bin` and `<index>.bin` files are backup WAD packages with a "Bk" header block, a TMD data block and encrypted contents using AES-128-CBC with the console-specific PRNG key and the content index as their IV (followed by 14 zeroes).
* However, `content.bin` files hold two leading blocks before the "Bk" header that are both encrypted using the SD key and the SD IV (which are not console specific):
    * A 0x640 byte-long title info header, which holds data such as title ID and a copy of the IMET header from the channel's `opening.bnr` (`00000000.app`).
    * A copy of the `/meta/icon.bin` file entry from the `opening.bnr` U8 archive, with a variable size.
* `content.bin` files also hold a trailing certificate area placed after the encrypted contents, which contains:
    * An ECSDA signature calculated over the whole backup WAD package area (using the console-specific ECC private key).
    * A copy of the console-specific ECC-B233 device certificate (also known as "NG" cert).
    * A title-issued ECC-B233 certificate (also known as "AP" cert), signed using the console-specific ECC private key. Its ECC public key is an ECDH shared secret generated with a custom ECC private key. The issuer title is always the System Menu (00000001-00000002).
* On the other hand, while `<index>.bin` files don't include any of the leading and trailing blocks from `content.bin` files, they are only allowed to hold a single encrypted content at a time, which index is used as part of the filename expressed in base 10 notation (e.g. `000.bin`).

Dependencies:
--------------

* [ninty-233](https://github.com/jbop1626/ninty-233) (licensed under GPLv3 or later) is used for ECDH data generation and ECSDA signing/verification.
* [mbedtls](https://tls.mbed.org) (licensed under Apache 2.0) is used for hash calculation and AES-CBC crypto operations.
* Keydata parsing based on code from [hactool](https://github.com/SciresM/hactool) (licensed under ISC).

License:
--------------

wad2bin is licensed under GPLv3 or (at your option) any later version.

Changelog:
--------------

**v0.6:**

* Added a hardcoded list with title IDs from DLCs that can be converted to the `<index>.bin` format. Any other DLCs will return an error. Thanks to this, it's no longer necessary to input a parent title ID.
* Fixed support for WAD packages with content data bigger than `U32_MAX` (0xFFFFFFFF).

**v0.5:**

Implemented bogus installable WAD package generation (header + certificate chain + ticket + TMD), saved at the provided output directory using the `<title_id>_bogus.wad` naming convention.

These bogus WAD packages can be used to install both ticket and TMD if needed, using regular WAD Managers. Errors such as -1022 can be safely ignored (e.g. content data isn't available in these WADs).

**v0.4:**

Fixed seeking after a missing content is detected while unpacking a DLC WAD.

**v0.3:**

Force the user to provide the full parent title ID for DLC WADs.

**v0.2:**

Added proper support for DLC WADs, even if they're incomplete (e.g. full TMD with missing content files).

**v0.1:**

Initial release.
