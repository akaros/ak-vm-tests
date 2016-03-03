Tests that run inside a Linux vm hosted on Akaros.
Right now you need to check the output of the tests manually (by watching your console or checking output files).

Copy the `GO` script to the directory of your clone of `rminnich/linux:last_known_good` (or `mtaufen/linux`, if `rminnich/linux` isn't up to date yet.)

Then do the following to create an initramfs source directory:

Starting from `.../linux`:
```
    mkdir initramfs_source_dir
    cd initramfs_source_dir
    sudo cpio -i < ../initramfs.cpio
```

Then:

`bash GO` to build the tests, add them to the initramfs, and build Linux. `GO` relies on `DIT`.