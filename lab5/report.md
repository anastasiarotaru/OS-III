# Operating Systems – Lab 5: The File System with Your Own Hands (Linux)

**Student:** Rotaru Anastasia, FAF-242
**Machine:** Linux laptop, normal user (not root). Work folder: `~/os-lab5`

## 1. The tree and the file types

### 1.1 Commands and outputs

```text
$ ls -ld /etc /etc/passwd /bin /dev/null /tmp
[paste the output here]

$ file /usr/bin/ls /etc/passwd /bin
[paste the output here]

$ du -sh /var/log 2>/dev/null ; du -sh /usr 2>/dev/null
74M     /var/log
6.3G    /usr
```

### 1.2 Observations

1. **File types from the first character of `ls -ld`:**
   - `/etc`: `d`, directory.
   - `/etc/passwd`: `-`, regular file.
   - `/bin`: `l`, symbolic link.
   - `/dev/null`: `c`, character device.
   - `/tmp`: `d`, directory with the sticky bit (`t`).
2. **What `/bin` is.** On modern Linux distributions `/bin` is a symbolic link to `/usr/bin`. This follows the usrmerge layout, where all core binaries are kept under `/usr`.
3. **Devices in `/dev`.** `/dev/null` is a character device (`c`), which passes data as a stream of single bytes. `/dev/sda` is a block device (`b`), which transfers data through the kernel page cache in fixed-size blocks.
4. **Which is bigger, `/var/log` or `/usr`.** `/usr` uses 6.3 GB and `/var/log` uses 74 MB, so `/usr` is roughly 85 times larger. `/usr` contains the programs, shared libraries and documentation, while `/var/log` only stores log files.

## 2. Inodes and links

### 2.1 An inode, and a name that is not part of it

```text
$ echo hello > notes.txt
$ stat notes.txt
  File: notes.txt
  Size: 6               Blocks: 8          IO Block: 4096   regular file
Device: 8,2    Inode: 1574449     Links: 1
Access: (0664/-rw-rw-r--)  Uid: ( 1000/ vitalie)   Gid: ( 1000/ vitalie)
$ ls -li notes.txt
[paste the output here: not shown in the earlier report]
```

### 2.2 Hard link, then remove the "original"

```text
$ ln notes.txt hard.txt
$ ls -li notes.txt hard.txt
1574449 -rw-rw-r-- 2 vitalie vitalie 6 Oct 12 12:49 hard.txt
1574449 -rw-rw-r-- 2 vitalie vitalie 6 Oct 12 12:49 notes.txt
$ rm notes.txt
$ cat hard.txt ; ls -li hard.txt
hello
1574449 -rw-rw-r-- 1 vitalie vitalie 6 Oct 12 12:49 hard.txt
```

### 2.3 Symbolic link, then remove its target

```text
$ ln -s hard.txt soft.txt
$ ls -li hard.txt soft.txt
1574449 -rw-rw-r-- 1 vitalie vitalie 6 Oct 12 12:49 hard.txt
1576266 lrwxrwxrwx 1 vitalie vitalie 8 Oct 12 12:49 soft.txt -> hard.txt
$ cat soft.txt
hello
$ rm hard.txt
$ cat soft.txt ; ls -l soft.txt
cat: soft.txt: No such file or directory
lrwxrwxrwx 1 vitalie vitalie 8 Oct 12 12:49 soft.txt -> hard.txt
```

### 2.4 Directories have links too

```text
$ mkdir d ; ls -ld d
drwxrwxr-x 2 vitalie vitalie 4096 Oct 12 12:49 d
$ mkdir d/x d/y ; ls -ld d
drwxrwxr-x 4 vitalie vitalie 4096 Oct 12 12:49 d
```

### 2.5 Size is not space

```text
$ echo x > one.txt ; ls -l one.txt ; du -h one.txt
-rw-rw-r-- 1 vitalie vitalie 2 Oct 12 12:49 one.txt
4.0K    one.txt
$ truncate -s 1G big.img ; ls -lh big.img ; du -h big.img
-rw-rw-r-- 1 vitalie vitalie 1.0G Oct 12 12:49 big.img
0       big.img
$ rm big.img
```

### 2.6 Observations

1. **Inode and link count in `stat`.** The inode is `1574449`, which is the unique number of the file in its device (`8,2`). The `Links` field (`1`) tells how many names point to this inode.
2. **Why `hard.txt` could still be read after `rm notes.txt`.** Both names pointed to the same inode. `rm` calls `unlink()`, which removes one directory entry and decrements the link count from 2 to 1. The data blocks stay in place because the count is still non-zero, so `rm` does not erase the data. It only removes a name.
3. **Why `cat soft.txt` failed.** `soft.txt` is a symbolic link. Its inode (`1576266`) stores the text path `hard.txt`, and not the data. After `hard.txt` was removed, the path can no longer be resolved, so the read fails. The size of `soft.txt` is 8 bytes, because the string `hard.txt` has 8 characters.
4. **Links of a new directory.** A new directory `d` has 2 links: the entry in the parent directory and its own `.` entry. Each subdirectory (`d/x` and `d/y`) has a `..` entry that points back to `d`. Two subdirectories add two more links, so the count becomes 4.
5. **Why a 2-byte file uses 4.0K and a 1 GB file uses 0.** The file system allocates disk space in blocks of 4096 bytes, so a 2-byte file still occupies one full block. `big.img` is a sparse file. `truncate` sets its size to 1 GB in the inode without allocating data blocks. Blocks are allocated only when data is written, so `du` reports 0.

## 3. Permissions and special bits

### 3.1 Files

```text
$ chmod 000 s.txt ; cat s.txt
cat: s.txt: Permission denied
$ chmod u+r s.txt ; cat s.txt
secret
$ umask
0002
$ touch new.txt ; mkdir newdir ; ls -ld new.txt newdir
-rw-rw-r-- 1 vitalie vitalie    0 Oct 12 12:49 new.txt
drwxrwxr-x 2 vitalie vitalie 4096 Oct 12 12:49 newdir
```

### 3.2 Directories: predict, then run

```text
$ mkdir box ; echo hi > box/f
$ chmod u=r,go= box ; ls box ; cat box/f
f
cat: box/f: Permission denied
$ chmod u=x,go= box ; ls box ; cat box/f
ls: cannot open directory 'box': Permission denied
hi
$ chmod 755 box
```

### 3.3 Special bits and other people's files

```text
$ ls -l /usr/bin/passwd ; ls -ld /tmp
-rwsr-xr-x 1 root root 93640 Feb  2  2026 /usr/bin/passwd
drwxrwxrwt 15 root root   360 Oct 12 12:46 /tmp
$ cat /etc/shadow
cat: /etc/shadow: Permission denied
$ ls -l /etc/shadow
-rw-r----- 1 root shadow 1335 Oct  5 13:09 /etc/shadow
```

### 3.4 Observations

1. **Permissions as rwx strings:**
   - `600` = `rw-------`. Read and write for the owner only. Used for private keys (for example `~/.ssh/id_rsa`).
   - `644` = `rw-r--r--`. Owner can write, everyone can read. Used for configuration files such as `/etc/resolv.conf`.
   - `755` = `rwxr-xr-x`. Owner has full access, everyone can read and execute. Used for public programs such as `/usr/bin/ls`.
   - `700` = `rwx------`. Full access for the owner only. Used for private directories such as `~/.ssh`.
2. **Why ownership does not help with `chmod 000`.** The permission check uses the class of the user that matches the file. As the owner, the user bits (`---`) apply, so read access is denied. Ownership gives the right to change the permissions with `chmod`, but it does not give read access automatically.
3. **umask and the new files.** The umask is `0002`. A new file starts from `0666` and a new directory from `0777`, and the umask removes the write bit for others:
   - file: `0666 AND NOT 0002 = 0664` gives `-rw-rw-r--`;
   - directory: `0777 AND NOT 0002 = 0775` gives `drwxrwxr-x`.
4. **The box experiment.** `r` on a directory allows listing its names (`ls box` works), but without `x` the files cannot be accessed, so `cat box/f` fails. `x` on a directory allows traversal and access to a file whose name is known (`cat box/f` works), but without `r` the listing is forbidden (`ls box` fails).
5. **The `s` and `t` bits.** The `s` in `/usr/bin/passwd` is the setuid bit. When `passwd` runs, it has the effective user ID of its owner, `root`. This lets a normal user update their password, which is stored in `/etc/shadow`, a file that only root can write. The `t` in `/tmp` is the sticky bit. In a world-writable directory it allows a user to delete or rename only the files they own.

## 4. Disks and the kernel as files

### 4.1 Commands and outputs

```text
$ lsblk
NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
sda      8:0    0    42G  0 disk
+-sda1   8:1    0     1M  0 part
+-sda2   8:2    0    42G  0 part /

$ mount | grep " / "
/dev/sda2 on / type ext4 (rw,relatime)

$ df -hm /
Filesystem     1M-blocks  Used Available Use% Mounted on
/dev/sda2          42027  9159     30702  23% /

$ grep -c processor /proc/cpuinfo
4

$ head -3 /proc/meminfo
MemTotal:        3479648 kB
MemFree:          404992 kB
MemAvailable:    2035864 kB

$ head -5 /proc/self/status
Name:   head
Umask:  0002
State:  R (running)
```

### 4.2 Observations

1. **Partitions and the root file system.** The disk is `sda`, 42 GB. It has two partitions: `sda1` (1 MB) and `sda2` (42 GB). `sda2` is mounted at `/` and uses the `ext4` file system.
2. **How full the root file system is.** 9159 MB of 42027 MB are used, which is 23%.
3. **CPU cores and RAM according to `/proc`.** The laptop has 4 processors (cores) and 3479648 kB of RAM, about 3.4 GB.
4. **The name in `/proc/self/status`.** It shows `head`. `/proc/self` is a link that resolves to the process that opens it. The `head` command opened the file, so the kernel reports the information about `head`.

## 5. Stretch goals (+1)

### 5.1 Stretch A: magic numbers

```text
$ gzip -c /etc/passwd > notes.txt ; file notes.txt ; xxd notes.txt | head -1
notes.txt: gzip compressed data, was "passwd", last modified: Mon Oct  5 13:09:51 2026, from Unix, original size modulo 2^32 2820
00000000: 1f8b 0808 9fa1 c36a 0003 7061 7373 7764  .......j..passwd
```

The first two bytes `1f 8b` are the magic number of gzip streams (RFC 1952). The `file` command reads these leading bytes and compares them with its magic database, so it can recognise the file type even if the name has no extension.

### 5.2 Stretch C: deleting a file that is still open

```text
$ truncate -s 10M bigfile ; exec 3< bigfile ; rm bigfile ; ls -l /proc/$$/fd
lr-x------ 1 vitalie vitalie 64 Oct 12 12:49 3 -> '/home/vitalie/os-lab5/bigfile (deleted)'
$ exec 3<&-
```

`rm` removes the name `bigfile`, but descriptor 3 still refers to the inode, so the 10 MB stay allocated. The space is freed only when the last descriptor is closed (`exec 3<&-`).

## 6. Conclusion

This laboratory work inspected the Linux file system directly from the command line. Several points stood out:
- Directory entries are names that point to inodes. The data and metadata belong to the inode, not to the name.
- A hard link shares the inode, and a symbolic link stores a path. Removing the target breaks only the symbolic link.
- Deleting a name frees the data only when the link count is zero and no process has the file open.
- Permissions are checked by user class (owner, group, others). For a directory, `r` lists names and `x` allows access to entries.
- `/proc` exposes kernel and hardware information as ordinary files.
