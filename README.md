Tests that run inside a Linux vm hosted on Akaros.
Right now you need to check the output of the tests manually (by watching your console or checking output files).

Clone this repo into your `linux/` directory, which should itself be a clone of `rminnich/linux:last_known_good` or `rminnich/linux/the-future`.

Copy the `GO` script to the root directory of your clone of `rminnich/linux:last_known_good` or `rminnich/linux/the-future`.

Then do the following to create an initramfs source directory:

Starting from `.../linux`:
```
    mkdir initramfs_source_dir
    cd initramfs_source_dir
    sudo cpio -i < ../initramfs.cpio
```

Then:

`bash GO` to build the tests, add them to the initramfs, and build Linux. `GO` relies on `DIT`.

#License

Copyright 2016 Google Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.