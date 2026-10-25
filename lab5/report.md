# Lab 5 - The File System with Your Own Hands

Name: Rotaru Anastasia
Group: FAF-242
Environment: GitHub Codespaces (container on Ubuntu 22.04), user codespace

---

## Part 1 - The Tree and File Types

### 1.1 Output

```bash
$ ls -ld /etc /etc/passwd /bin /dev/null /tmp
lrwxrwxrwx  1 root root    7 Apr 22  2024 /bin -> usr/bin
crw-rw-rw-  1 root root 1, 3 Oct 25 21:41 /dev/null
drwxr-xr-x  1 root root 4096 Oct 22 08:58 /etc
-rw-r--r--  1 root root 1115 Sep 10 12:29 /etc/passwd
drwxr-xrwt+ 4 root root 4096 Oct 25 21:42 /tmp

$ ls -l /dev | head
crw-r--r-- 1 root root     10, 235 Oct 25 21:41 autofs
drwxr-xr-x 2 root root         120 Oct 25 21:41 bsg
crw------- 1 root root     10, 234 Oct 25 21:41 btrfs-control
lrwxrwxrwx 1 root root          11 Oct 25 21:41 core -> /proc/kcore
crw------- 1 root root     10, 261 Oct 25 21:41 cpu_dma_latency
crw------- 1 root root     10, 203 Oct 25 21:41 cuse
drwxr-xr-x 2 root root          60 Oct 25 21:41 dma_heap
drwxr-xr-x 2 root root          60 Oct 25 21:41 dri
crw------- 1 root root     10, 258 Oct 25 21:41 ecryptfs

$ ls -l /dev | grep '^b'
brw-rw---- 1 root disk      7,   0 Oct 25 21:41 loop0
brw-rw---- 1 root disk      7,   1 Oct 25 21:41 loop1
brw-rw---- 1 root disk      7,   2 Oct 25 21:41 loop2
brw-rw---- 1 root disk      7,   3 Oct 25 21:41 loop3
brw-rw---- 1 root disk      7,   4 Oct 25 21:41 loop4
brw-rw---- 1 root disk      7,   5 Oct 25 21:41 loop5
brw-rw---- 1 root disk      7,   6 Oct 25 21:41 loop6
brw-rw---- 1 root disk      7,   7 Oct 25 21:41 loop7
brw------- 1 root root      8,   1 Oct 25 21:41 root
brw-rw---- 1 root disk      8,   0 Oct 25 21:41 sda
brw-rw---- 1 root disk      8,   1 Oct 25 21:41 sda1
brw-rw---- 1 root disk      8,  14 Oct 25 21:41 sda14
brw-rw---- 1 root disk      8,  15 Oct 25 21:41 sda15
brw-rw---- 1 root disk      8,  16 Oct 25 21:41 sdb
brw-rw---- 1 root disk      8,  17 Oct 25 21:41 sdb1
brw-rw---- 1 root disk      8,  32 Oct 25 21:41 sdc
brw-rw---- 1 root disk      8,  33 Oct 25 21:41 sdc1
brw-rw---- 1 root cdrom    11,   0 Oct 25 21:41 sr0

$ file /usr/bin/ls /etc/passwd /bin
/usr/bin/ls: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=44900829b39271d878615772bb2a6ffa422f0891, for GNU/Linux[...]
/etc/passwd: ASCII text
/bin:        symbolic link to usr/bin

$ du -sh /var/log 2>/dev/null ; du -sh /usr 2>/dev/null
828K    /var/log
7.0G    /usr
```

### 1.2 Observations

- `/bin` is a symbolic link.
- `/dev/null` is a character device.
- `/etc` is a directory.
- `/etc/passwd` is a regular file.
- `/tmp` is a directory with the sticky bit and ACL attributes.

### 1.3 Explanation

#### What is `/bin` on this system?
`/bin` is a symbolic link that points to `usr/bin`.

#### Character and block devices in `/dev`
- Character device: `/dev/null`  
  It handles unbuffered text streams and processes data character by character.
- Block device: `/dev/sda1`  
  It handles buffered data in fixed-size blocks, which is typical for storage devices.

#### Which is bigger, `/var/log` or `/usr`?
`/usr` is much larger than `/var/log`.

- `/var/log`: about `828 KB`
- `/usr`: about `7.0 GB`

So `/usr` is larger by roughly 7 GB.

---

## Part 2 - Inodes and Links

### 2.1 Creating a file and checking inode information

#### Output

```bash
$ echo hello > notes.txt

$ stat notes.txt
  File: notes.txt
  Size: 6               Blocks: 8          IO Block: 4096   regular file
Device: 0,45    Inode: 1443212     Links: 1
Access: (0644/-rw-r--r--)  Uid: ( 1000/codespace)   Gid: ( 1000/codespace)
Access: 2026-10-25 21:49:05.959342107 +0000
Modify: 2026-10-25 21:49:05.959342107 +0000
Change: 2026-10-25 21:49:05.959342107 +0000
 Birth: 2026-10-25 21:49:05.959342107 +0000

$ ls -li notes.txt
1443212 -rw-r--r-- 1 codespace codespace 6 Oct 25 21:49 notes.txt
```

#### Observation
The inode number is `1443212`, and the number of hard links is `1`.

### 2.2 Hard link

#### Output

```bash
$ ln notes.txt hard.txt

$ ls -li notes.txt hard.txt
1443212 -rw-r--r-- 2 codespace codespace 6 Oct 25 21:49 hard.txt
1443212 -rw-r--r-- 2 codespace codespace 6 Oct 25 21:49 notes.txt

$ rm notes.txt

$ cat hard.txt ; ls -li hard.txt
hello
1443212 -rw-r--r-- 1 codespace codespace 6 Oct 25 21:49 hard.txt
```

#### Explanation
Both names point to the same inode, so deleting `notes.txt` does not destroy the file data immediately.  
`rm` removes only the directory entry; the inode and its data remain until the last link is removed.

### 2.3 Symbolic link

#### Output

```bash
$ ln -s hard.txt soft.txt

$ ls -li hard.txt soft.txt
1443212 -rw-r--r-- 1 codespace codespace 6 Oct 25 21:49 hard.txt
1443213 lrwxrwxrwx 1 codespace codespace 8 Oct 25 21:49 soft.txt -> hard.txt

$ cat soft.txt
hello

$ rm hard.txt

$ cat soft.txt ; ls -l soft.txt
cat: soft.txt: No such file or directory
lrwxrwxrwx 1 codespace codespace 8 Oct 25 21:49 soft.txt -> hard.txt
```

#### Explanation
A symbolic link stores a text path to another file, not the file data itself.  
The file `soft.txt` is exactly 8 bytes long because it contains the string `"hard.txt"`.

After `hard.txt` is removed, the symlink becomes a dangling symlink: it still exists, but its target no longer exists.

### 2.4 Directory links

#### Output

```bash
$ mkdir d ; ls -ld d
drwxr-xr-x 2 codespace codespace 4096 Oct 25 21:49 d

$ mkdir d/x d/y ; ls -ld d
drwxr-xr-x 4 codespace codespace 4096 Oct 25 21:49 d
```

#### Explanation
A new directory starts with 2 links:
- one from its entry in the parent directory
- one from the `.` entry inside itself

When subdirectories are created, each subdirectory contains a `..` entry pointing back to the parent directory.  
This adds 2 more links to the parent directory, so the count becomes 4.

### 2.5 Size is not space

#### Output

```bash
$ echo x > one.txt ; ls -l one.txt ; du -h one.txt
-rw-r--r-- 1 codespace codespace 2 Oct 25 21:49 one.txt
4.0K    one.txt

$ truncate -s 1G big.img ; ls -lh big.img ; du -h big.img
-rw-r--r-- 1 codespace codespace 1.0G Oct 25 21:49 big.img
0       big.img

$ rm big.img
```

#### Explanation
A 2-byte file still uses a whole 4 KB block on disk because filesystems allocate space in fixed-size blocks.

A 1 GB sparse file (`truncate -s 1G`) has a logical size of 1 GB but consumes almost no actual disk space until data is written.  
Therefore, `du` shows `0`.

### 2.6 Summary
- The inode number identifies the actual filesystem object.
- The `Links` count tells how many directory entries point to the same inode.
- `rm` deletes directory entries, not necessarily the underlying file.
- Symbolic links point to paths, while hard links point to the same inode.
- File size and disk usage are not the same thing.

---

## Part 3 - Permissions

### 3.1 Basic permission experiments

#### Output

```bash
$ echo secret > s.txt

$ chmod 600 s.txt ; ls -l s.txt
-rw------- 1 codespace codespace 7 Oct 25 21:49 s.txt

$ chmod 000 s.txt ; cat s.txt
cat: s.txt: Permission denied

$ chmod u+r s.txt ; cat s.txt
secret

$ umask ; touch new.txt ; mkdir newdir ; ls -ld new.txt newdir
0022
-rw-r--r-- 1 codespace codespace    0 Oct 25 21:49 new.txt
drwxr-xr-x 2 codespace codespace 4096 Oct 25 21:49 newdir
```

#### Explanation
When a file has permissions `000`, the owner still cannot read it because ownership does not override permission checking.  
The kernel checks permission bits before granting access.

The umask is `0022`. This subtracts permissions from the default values:
- regular files default to `666` → `644`
- directories default to `777` → `755`

Therefore:
- `new.txt` becomes `-rw-r--r--`
- `newdir` becomes `drwxr-xr-x`

### 3.2 Directory permission experiment

#### Output

```bash
$ mkdir box ; echo hi > box/f

$ chmod u=r,go= box ; ls box ; cat box/f
f
cat: box/f: Permission denied

$ chmod u=x,go= box ; ls box ; cat box/f
ls: cannot open directory 'box': Permission denied
hi

$ chmod 755 box
```

#### Explanation
The permission bits on directories work differently than on files.

- `r` on a directory allows reading the list of entries.
- `x` on a directory allows entering it and accessing its contents.

In the first case:
- `ls box` succeeded because read permission was granted.
- `cat box/f` failed because execute permission was absent.

In the second case:
- `ls box` failed because read permission was absent.
- `cat box/f` succeeded because the file could still be accessed by its full path, even without listing the directory.

### 3.3 Special permission bits

#### Output

```bash
$ ls -l /usr/bin/passwd ; ls -ld /tmp
-rwsr-xr-x 1 root root 64152 May 30  2024 /usr/bin/passwd
drwxr-xrwt+ 4 root root 4096 Oct 25 21:42 /tmp

$ cat /etc/shadow
cat: /etc/shadow: Permission denied

$ ls -l /etc/shadow
-rw-r----- 1 root shadow 603 Sep 10 12:29 /etc/shadow
```

#### Explanation
- `s` in `/usr/bin/passwd` stands for the SetUID bit.
- `t` in `/tmp` stands for the sticky bit.

`passwd` needs the SetUID bit so a normal user can run it with root privileges in order to update password-related files.  
The sticky bit on `/tmp` ensures that users can only delete or rename files they own inside the directory.

### 3.4 Permission examples

- `600` → `-rw-------`
- `644` → `-rw-r--r--`
- `755` → `-rwxr-xr-x`
- `700` → `-rwx------`

#### Real uses
- `600`: private SSH keys
- `644`: public-readable configuration files
- `755`: executables and scripts
- `700`: directories or scripts reserved for one user

---

## Part 4 - Disks and the Kernel as Files

### 4.1 Disk and filesystem information

#### Output

```bash
$ lsblk
NAME    MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
loop0     7:0    0    74M  1 loop 
loop1     7:1    0 115.3M  1 loop 
loop2     7:2    0  50.1M  1 loop 
loop3     7:3    0   257G  0 loop 
loop4     7:4    0    32G  0 loop /var/lib/containerd
                                  /home/vscode/.minikube
                                  /workspaces/.codespaces/.persistedshare
                                  /var/lib/docker
                                  /etc/hosts
                                  /etc/hostname
                                  /etc/resolv.conf
                                  /workspaces
loop5     7:5    0   500M  0 loop 
sda       8:0    0    30G  0 disk 
├─sda1    8:1    0  29.9G  0 part /workspaces/.codespaces/shared
│                                 /.codespaces/bin
│                                 /vscode
│                                 /usr/sbin/docker-init
├─sda14   8:14   0     4M  0 part 
└─sda15   8:15   0   106M  0 part 
sdb       8:16   0    45G  0 disk 
└─sdb1    8:17   0    45G  0 part /tmp
sdc       8:32   0   512G  0 disk 
└─sdc1    8:33   0   512G  0 part 
sr0      11:0    1   628K  1 rom  

$ df -h
Filesystem      Size  Used Avail Use% Mounted on
overlay          32G   12G   19G  38% /
tmpfs            64M     0   64M   0% /dev
shm              64M     0   64M   0% /dev/shm
/dev/root        29G   17G   13G  56% /vscode
/dev/loop4       32G   12G   19G  38% /workspaces
/dev/sdb1        44G  2.0G   40G   5% /tmp

$ df -h /
Filesystem      Size  Used Avail Use% Mounted on
overlay          32G   12G   19G  38% /

$ mount | grep " / "
overlay on / type overlay (rw,relatime,lowerdir=/var/lib/docker/overlay2/l/KSEMGVJEKSYD4C56ITBY44F7XJ:/var/lib/docker/overlay2/l/ZJQ5245WW4Q7A7CYK3P2SZYIMD:/var/lib/docker/overlay2/l/QAXJCL2DNKHR5CCD6[...]
```

### 4.2 Kernel and process information

#### Output

```bash
$ grep -c processor /proc/cpuinfo
2

$ head -3 /proc/meminfo
MemTotal:        8131872 kB
MemFree:          186508 kB
MemAvailable:    6000536 kB

$ head -5 /proc/self/status
Name:   head
Umask:  0022
State:  R (running)
Tgid:   5765
Ngid:   0

$ ls -l /proc/self/fd
total 0
lrwx------ 1 codespace codespace 64 Oct 25 21:49 0 -> /dev/pts/1
l-wx------ 1 codespace codespace 64 Oct 25 21:49 1 -> pipe:[106563]
l-wx------ 1 codespace codespace 64 Oct 25 21:49 2 -> pipe:[106563]
lr-x------ 1 codespace codespace 64 Oct 25 21:49 3 -> /proc/5766/fd
```

### 4.3 Interpretation

#### Root filesystem
The root filesystem is mounted as:

- `/` → `overlay`
- file system type: `overlay`

The root filesystem is about `38% full`:
- total: `32G`
- used: `12G`
- free: `19G`

#### CPU and memory
- CPU cores: `2`
- RAM: `8,131,872 kB` ≈ `7.76 GB`

#### What does `/proc/self/status` show?
The process name shown is `head`, because `/proc/self` refers to the current process.  
The command being run at that moment was:

```bash
head -5 /proc/self/status
```

Thus, the kernel reports the current process name as `head`.

---

## Bonus

### Bonus A - Magic numbers in gzip

#### Output

```bash
$ gzip -c /etc/passwd > notes2.txt ; file notes2.txt ; xxd notes2.txt | head -1
notes2.txt: gzip compressed data, was "passwd", last modified: Thu Sep 10 12:29:43 2026, from Unix, original size modulo 2^32 1115
00000000: 1f8b 0808 b7a2 a26a 0003 7061 7373 7764  .......j..passwd
```

#### Explanation
The first bytes are `1f 8b`, which are the gzip magic numbers.  
These bytes identify the file as gzip-compressed data.

### Bonus B - Deleted file still open

#### Output

```bash
$ echo ceva > bigfile; exec 3< bigfile; rm bigfile; ls -l /proc/$$/fd; exec 3<&-
lr-x------ 1 codespace codespace 64 Oct 25 21:49 3 -> /home/codespace/os-lab5/bigfile (deleted)
```

#### Explanation
Even after the file is removed from the directory, the process still has an open file descriptor to it.  
The kernel keeps the inode alive until all descriptors are closed, so the deleted file still exists as long as the process keeps it open.

---

## Conclusion

This laboratory work helped me understand the fundamental structure and behavior of the Linux filesystem. I learned how to distinguish file types, how directories and inodes work, how permissions are enforced, and how the kernel exposes system information through pseudo-filesystems such as `/proc`.

The most important ideas from this lab were:
- files are not just names; they are connected to inodes,
- hard links and symbolic links behave differently,
- permissions are enforced by the kernel,
- `/proc` exposes live system state in a filesystem-like way,
- storage devices and mount points are represented as files and directories.

This knowledge is essential for understanding how Linux organizes data, access, and resources at the operating system level.
