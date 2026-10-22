# Operating Systems – Lab 6: Inside the xv6 File System

**Student:** Rotaru Anastasia, FAF-242
**Tree:** `~/xv6-riscv` (xv6 in QEMU)

## Part 1 – Read what xv6 shows you

### Commands and output

```text
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2441
cat            2 3 35376
...
console        3 25 0
$ mkdir a
$ ls a
.              1 26 32
..             1 1 1024
```

Each line is: name, type, inode number, size in bytes. The listing above is shortened; the full output is in the report.

### Observations

1. **Types 1, 2 and 3 (`kernel/stat.h`):** `T_DIR` (1) is a directory, `T_FILE` (2) is a regular file, and `T_DEVICE` (3) is a device file such as `console`.
2. **Root inode and `.` / `..`:** the root directory has inode `1` (`ROOTINO`). It has no parent, so both `.` and `..` point to inode 1.
3. **Size of an entry:** a `struct dirent` has a 2-byte inode number (`ushort inum`) and a 14-byte name (`char name[DIRSIZ]`), so one entry is **16 bytes**. The empty directory `a` has size 32, so it contains **2 entries**: `.` and `..`.

## Part 2 – Write mystat

`user/mystat.c` calls `stat()` for each argument and prints the name, type, inode, number of links and size. On error it prints a message to file descriptor 2 and continues with the next argument.

### Hard link trace

```text
$ mystat README . console
README: file, inode 2, links 1, size 2441 bytes
.: directory, inode 1, links 1, size 1024 bytes
console: device, inode 27, links 1, size 0 bytes
$ ln README r2
$ mystat README r2
README: file, inode 2, links 2, size 2441 bytes
r2: file, inode 2, links 2, size 2441 bytes
$ rm README
$ mystat r2
r2: file, inode 2, links 1, size 2441 bytes
$ cat r2 | wc
48 336 2441
```

> **Check:** `ls` shows `console` with inode 25, but this `mystat` run shows inode 27. Run `ls` and `mystat console` again on the same disk image and correct whichever value is wrong.

### Observations

1. **Outputs after `ln` and after `rm`.** After `ln README r2`, both names have inode 2 and `links 2`. The type, inode and size are the same for both names. After `rm README`, the name `README` disappears, `r2` still has inode 2, and `links` drops to 1. The data is not removed, because `cat r2 | wc` still reads all 2441 bytes.
2. **Where the link count is decremented.** In `kernel/sysfile.c`, `sys_unlink` decrements the count with `ip->nlink--;` and writes the inode with `iupdate(ip)`. The data blocks are not freed there. They are freed in `iput()` in `kernel/fs.c`, when the in-memory reference count drops to 0 and `ip->nlink == 0`. Then `itrunc(ip)` releases all the blocks. (Check these line numbers in your tree.)
3. **Fields of `struct dinode` (`kernel/fs.h`).** An xv6 inode has no permission bits, owner UID/GID or timestamps. Its fields are: `type` (file type), `major` and `minor` (device numbers), `nlink` (number of directory entries that point to the inode), `size` (file size in bytes), and `addrs[NDIRECT+1]` (direct block addresses and one indirect block address).

## Part 3 – Measure the limits

### Header constants and disk layout

```text
$ grep -n "define BSIZE\|define NDIRECT\|define NINDIRECT\|define DIRSIZ" kernel/fs.h
5:#define BSIZE   1024
26:#define NDIRECT 12
27:#define NINDIRECT (BSIZE / sizeof(uint))
54:#define DIRSIZ 14
$ make clean ; make fs.img 2>&1 | grep nmeta
nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000
```

### Long directory name

```text
$ mkdir averyveryverylongname
$ ls
averyveryveryl
[paste the full ls output, with the inode numbers, here]
```

### Observations

1. **Long name.** `ls` shows `averyveryveryl`, which is 14 characters. The constant `DIRSIZ` (14, in `kernel/fs.h`) sets the maximum length of a name in a directory entry. Longer names are cut.
2. **Largest file.** An indirect block holds `NINDIRECT = BSIZE / sizeof(uint) = 1024 / 4 = 256` block numbers. The file can use `NDIRECT + NINDIRECT = 12 + 256 = 268` blocks, so its maximum size is `268 × 1024 = 274 432` bytes (about 268 KB).
3. **Disk blocks.** The `nmeta` line shows 47 metadata blocks: 1 boot block, 1 superblock, 31 log blocks, 13 inode blocks and 1 bitmap block. The disk has 2000 blocks in total, so `2000 − 47 = 1953` blocks are available for file and directory data.

## Part 4 – Write find

`user/find.c` takes a directory and a name, and prints every path under the directory whose last component is that name. It reads the directory entries with `read()`, skips free slots (`inum == 0`) and the entries `.` and `..`, builds the full path in a buffer, and calls itself for each subdirectory. It closes every directory it opens.

### Test on nested directories

```text
$ mkdir a/b
$ echo x > a/b/r2
$ echo y > a/r2
$ find . r2
./r2
./a/b/r2
./a/r2
$ find a b
a/b
```

The order of the lines may differ on your system. The lab sheet also asks for `ln cat r2` before `find . r2`. Run the exact sequence from the sheet and paste the output here.

### Stretch: inode numbers

```text
$ find . r2 -n
./r2 2
./a/b/r2 31
./a/r2 32
```

`./r2` has inode 2, which is the same inode as `README` had before it was removed, so `./r2` is a hard link to that file. `./a/b/r2` and `./a/r2` have different inodes, so they are separate files with the same name. Repeat this with `ln cat r2` and say which of the three `r2` files is the same file as `cat`.

## Conclusion

The lab showed how xv6 stores files and directories. A directory is a table of 16-byte entries that map names to inode numbers. Each inode holds the type, the link count, the size and the block addresses. A hard link increments `nlink`, and the data is freed only when the link count and the reference count both drop to zero. The header constants `BSIZE`, `NDIRECT`, `NINDIRECT` and `DIRSIZ` set the limits of file size and name length. Writing `mystat` and `find` showed how user programs use `stat()`, `read()` and `open()` on directories.
