Lab 5 - The File System with Your Own Hands

Name: Rotaru Anastasia

Group: FAF-242

Environment: GitHub Codespaces (container on Ubuntu 22.04), user codespace

Part 1 - The Tree and File Types

Output

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
/usr/bin/ls: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=44900829b39271d878615772bb2a6ffa422f0891, for GNU/Linux 3.2.0, stripped
/etc/passwd: ASCII text
/bin:        symbolic link to usr/bin

$ du -sh /var/log 2>/dev/null ; du -sh /usr 2>/dev/null
828K    /var/log
7.0G    /usr


Observe

Type of each path (from the first letter of ls -l):

/bin: l (Symbolic link)

/dev/null: c (Character device)

/etc: d (Directory)

/etc/passwd: - (Regular file)

/tmp: d (Directory with sticky bit and ACL)

What is /bin on your system: a directory or a link, and to where?

/bin is a symbolic link that targets usr/bin.

A character device and a block device in /dev:

Character Device: /dev/null (handles unbuffered data streams character-by-character).

Block Device: /dev/sda1 (handles buffered data transfers in fixed blocks for storage devices).

Which is bigger, /var/log or /usr, and by roughly how much?

/usr is considerably larger than /var/log. While /var/log takes up 828 KB, /usr uses 7.0 GB, making /usr larger by about 7 gigabytes.

Part 2 - Inodes and Links

2.1 Output

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


2.2 Output (Hard Link)

$ ln notes.txt hard.txt

$ ls -li notes.txt hard.txt
1443212 -rw-r--r-- 2 codespace codespace 6 Oct 25 21:49 hard.txt
1443212 -rw-r--r-- 2 codespace codespace 6 Oct 25 21:49 notes.txt

$ rm notes.txt

$ cat hard.txt ; ls -li hard.txt
hello
1443212 -rw-r--r-- 1 codespace codespace 6 Oct 25 21:49 hard.txt


2.3 Output (Symbolic Link)

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


2.4 Output (Directory Links)

$ mkdir d ; ls -ld d
drwxr-xr-x 2 codespace codespace 4096 Oct 25 21:49 d

$ mkdir d/x d/y ; ls -ld d
drwxr-xr-x 4 codespace codespace 4096 Oct 25 21:49 d


2.5 Output (Size is not Space)

$ echo x > one.txt ; ls -l one.txt ; du -h one.txt
-rw-r--r-- 1 codespace codespace 2 Oct 25 21:49 one.txt
4.0K    one.txt

$ truncate -s 1G big.img ; ls -lh big.img ; du -h big.img
-rw-r--r-- 1 codespace codespace 1.0G Oct 25 21:49 big.img
0       big.img

$ rm big.img


Observe

Which number in stat is the inode, and which one tells how many names the file has?

Inode number: 1443212 (listed as Inode: 1443212).

Number of names (hard link count): 1 (listed as Links: 1).

After rm notes.txt, why could you still read hard.txt? What does rm really remove?

hard.txt references the exact same inode (1443212) and data blocks on disk as notes.txt. Running rm removes only the filename entry notes.txt from the directory structure and decreases the inode's link counter from 2 to 1. The data on disk remains accessible through hard.txt until all link references are removed.

Why did cat soft.txt fail at the end, although soft.txt still exists? How big is soft.txt, and why exactly that size?

soft.txt stores only the textual path string pointing to hard.txt. When hard.txt was erased, soft.txt turned into a dangling symlink targeting a non-existent file.

soft.txt is 8 bytes long because its contents consist solely of the 8-character string "hard.txt".

A new directory has 2 links, and 4 after you create two subdirectories. Explain where each link comes from.

A freshly created directory d has 2 links: one from its entry inside the parent folder, and another from its internal . (self) link.

Creating subdirectories d/x and d/y introduces a .. (parent reference) inside both x and y pointing back to d, increasing d's link count by 2 (raising it to 4).

Why does a 2-byte file use 4.0K, and a 1 GB file use 0 on disk?

one.txt uses 4.0K because block storage systems allocate space in minimal fixed block sizes (4 KB), so even a 2-byte file consumes one full block.

big.img uses 0 bytes of real disk space because truncate generates a sparse file—a file structure that tracks metadata boundaries without assigning actual disk storage blocks until data is written.

Part 3 - Permissions

3.1 Output

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


3.2 Output (Directories)

$ mkdir box ; echo hi > box/f

$ chmod u=r,go= box ; ls box ; cat box/f
f
cat: box/f: Permission denied

$ chmod u=x,go= box ; ls box ; cat box/f
ls: cannot open directory 'box': Permission denied
hi

$ chmod 755 box


3.3 Output (Special Bits)

$ ls -l /usr/bin/passwd ; ls -ld /tmp
-rwsr-xr-x 1 root root 64152 May 30  2024 /usr/bin/passwd
drwxr-xrwt+ 4 root root 4096 Oct 25 21:42 /tmp

$ cat /etc/shadow
cat: /etc/shadow: Permission denied

$ ls -l /etc/shadow
-rw-r----- 1 root shadow 603 Sep 10 12:29 /etc/shadow


Observe

Write 600, 644, 755 and 700 as rwx strings, and give one real use for each.

600 (-rw-------): Read and write permissions restricted strictly to the file owner. Use: Private keys (e.g., ~/.ssh/id_rsa).

644 (-rw-r--r--): Read and write permissions for the owner; read-only access for group members and others. Use: Public site resources or configuration files.

755 (-rwxr-xr-x): Full access for the owner; read and execution permissions for group members and others. Use: Shell scripts or binary executables in system directories.

700 (-rwx------): Read, write, and execute permissions granted exclusively to the owner. Use: Confidential operational scripts or user-private directory structures.

With chmod 000 you own the file but cannot read it. Why does ownership not help?

File ownership does not bypass permission checks; it only determines which set of permission bits (User, Group, or Others) applies when a process attempts to access the file. When you access a file you own, the operating system selects the owner triplet (the first three bits). Because chmod 000 sets the owner bits to ---, access is denied despite ownership. The only identity that bypasses standard rwx permission bits entirely is root. This results in an explicit block on access.

What is your umask, and how does it explain the permissions of new.txt and newdir?

The umask setting is 0022. It subtracts specific permission bits from default bases during item creation:

Regular files begin with base 666 (rw-rw-rw-). Masking 0022 leaves 644 (-rw-r--r--), which matches new.txt.

Directories begin with base 777 (rwxrwxrwx). Masking 0022 leaves 755 (drwxr-xr-x), which matches newdir.

Explain the box experiment: what does r allow on a directory, and what does x allow?

r (Read): Allows reading the directory table to display file names within it (e.g., using ls).

x (Execute/Search): Allows entering the directory, traversing through its path, and accessing metadata or file contents inside it.

In the experiment:

Under chmod u=r, listing directory contents (ls box) succeeded, but accessing box/f failed without directory traversal permission (x).

Under chmod u=x, listing directory contents (ls box) failed, but reading target file contents (cat box/f) succeeded by providing the direct file path.

What do the s in /usr/bin/passwd and the t in /tmp mean, and why does passwd need the s bit?

The s (SetUID) bit on /usr/bin/passwd tells the kernel to execute the command under the rights of the file's owner (root). This capability allows non-root users to modify their user credentials saved in elevated security files like /etc/shadow.

The t (Sticky Bit) on /tmp ensures that files inside this shared space can only be deleted or renamed by the file owner or the root administrative account.

Part 4 - Disks and the Kernel as Files

Output

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

$ lsblk -f
NAME    FSTYPE FSVER LABEL UUID FSAVAIL FSUSE% MOUNTPOINTS
loop0                                          
loop1                                          
loop2                                          
loop3                                          
loop4                             18.6G    35% /var/lib/containerd
                                               /home/vscode/.minikube
                                               /workspaces/.codespaces/.persistedshare
                                               /var/lib/docker
                                               /etc/hosts
                                               /etc/hostname
                                               /etc/resolv.conf
                                               /workspaces
loop5                                          
sda                                            
├─sda1                            12.9G    55% /workspaces/.codespaces/shared
│                                              /.codespaces/bin
│                                              /vscode
│                                              /usr/sbin/docker-init
├─sda14                                        
└─sda15                                        
sdb                                            
└─sdb1                            39.7G     5% /tmp
sdc                                            
└─sdc1                                         
sr0                                            

$ mount | grep " / "
overlay on / type overlay (rw,relatime,lowerdir=/var/lib/docker/overlay2/l/KSEMGVJEKSYD4C56ITBY44F7XJ:/var/lib/docker/overlay2/l/ZJQ5245WW4Q7A7CYK3P2SZYIMD:/var/lib/docker/overlay2/l/QAXJCL2DNKHR5CCD6DQC2LMWOI:/var/lib/docker/overlay2/l/O76VGHRRHDS6BJMZGYYOYU6J6E:/var/lib/docker/overlay2/l/COQXJ2XREH624H74URK4CVGXUC:/var/lib/docker/overlay2/l/ML2O6SDGWCC5TJX2EFYBST3N5B:/var/lib/docker/overlay2/l/ELYOP56KWROTLPAQSMLU6IKEBB:/var/lib/docker/overlay2/l/FPW77L3ZCBZNRRW7GTLVML4HMO:/var/lib/docker/overlay2/l/F6D5ZMBBNNCIETGPM3RUZK4F7W:/var/lib/docker/overlay2/l/2QDNJ3WTTHUB2EGTBWIKYUGXE3:/var/lib/docker/overlay2/l/TFSHNOLTZT6J4G5HVHJPS2O4A2:/var/lib/docker/overlay2/l/IESA3SYTHMX3WJRCPIKGLGQFBW:/var/lib/docker/overlay2/l/WEUKK4JTM33YFTNVNQBHXREXAU:/var/lib/docker/overlay2/l/3QVGGB5BKGQBNZVQY6V75HDJ74:/var/lib/docker/overlay2/l/EXPM3Y4PNKW5AIB3KU3S2DJY77:/var/lib/docker/overlay2/l/ER5HPNE5VXZYUFVCFRMM2IF3A3:/var/lib/docker/overlay2/l/T5TITQE2I2XTXY37YAVPDNABKG:/var/lib/docker/overlay2/l/6GL5DTSKK6GHFESR4E423ZVT7L:/var/lib/docker/overlay2/l/ZIQ7VJCSZ53KPD5JEM3CMIV2R7:/var/lib/docker/overlay2/l/YMAHWBML5WSQKSJYAA23EMPQMT:/var/lib/docker/overlay2/l/ZZMPRI3FNYA3IQLTOYXLCLMUIN:/var/lib/docker/overlay2/l/R5A7XXBOEGRC2JPFWKXWDEQ7CT:/var/lib/docker/overlay2/l/4NDQD2K5BPLULZZEIJQZGF5FFQ:/var/lib/docker/overlay2/l/MTPUTL7KJXDXPTSPR2ZMJST5M5:/var/lib/docker/overlay2/l/3KXALDXVJM5UU7HDAYRK6QUJVW:/var/lib/docker/overlay2/l/Z3GVQMJJNJZQJGDAN53PLTOIE3:/var/lib/docker/overlay2/l/YTOZCMPAF2PQB5EGJLYTZHQ4GD:/var/lib/docker/overlay2/l/JPRFBSZMTSALEBNBX77F5VMYPE:/var/lib/docker/overlay2/l/WTS3UUQI4A6HROUZPMI6KCWJU3:/var/lib/docker/overlay2/l/JBQX3Q2O3IFFEVEFDAVHS4MTFG:/var/lib/docker/overlay2/l/CAZGISGVDPUKDCJ4USIA5BQC3W:/var/lib/docker/overlay2/l/OJPTCDH5VWMIMLQ7Q5JPJBRWON,upperdir=/var/lib/docker/overlay2/1f3cfda4c0954e57fb15198ce26bdd23dcde74e6a99890a40b157e3b9e909afe/diff,workdir=/var/lib/docker/overlay2/1f3cfda4c0954e57fb15198ce26bdd23dcde74e6a99890a40b157e3b9e909afe/work,nouserxattr)

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


Observe

Draw your disk: disk names, partitions, which one is mounted at /, and with what file system type.

[Storage Layout Diagram]

+-------------------------------------------------------------------------+
| Virtual Root File System (Container Layer)                              |
|   - Mounted at: /                                                       |
|   - File System Type: overlay (32 GB)                                   |
+-------------------------------------------------------------------------+

Physical / Virtual Disk Drives:

+-- sda (30 GB Disk)
|   +-- sda1  (29.9 GB) --> Mounted at /vscode, /workspaces/.codespaces/shared
|   +-- sda14 (4.0 MB)  --> Partition (unmounted)
|   +-- sda15 (106 MB)  --> Partition (unmounted)
|
+-- sdb (45 GB Disk)
|   +-- sdb1  (45 GB)   --> Mounted at /tmp
|
+-- sdc (512 GB Disk)
|   +-- sdc1  (512 GB)  --> Unmounted partition
|
+-- loop4 (32 GB Virtual Loop Device)
    +-- Mounted at /workspaces, /var/lib/docker, /etc/hosts


How full is your root file system?

The root filesystem (overlay) is 38% full, using 12 GB out of 32 GB, with 19 GB of available capacity remaining.

How many CPU cores and how much RAM does the system have, according to /proc?

CPU Cores: 2 virtual processors (/proc/cpuinfo).

RAM: Total memory (MemTotal) is 8,131,872 kB (~7.76 GB).

/proc/self/status showed the name of a program. Which one, and why that one?

The program name reported is head. This occurs because /proc/self dynamically evaluates to the process reading the information at runtime—which was head (head -5 /proc/self/status).

Bonus

(a) Magic numbers

$ gzip -c /etc/passwd > notes2.txt ; file notes2.txt ; xxd notes2.txt | head -1
notes2.txt: gzip compressed data, was "passwd", last modified: Thu Sep 10 12:29:43 2026, from Unix, original size modulo 2^32 1115
00000000: 1f8b 0808 b7a2 a26a 0003 7061 7373 7764  .......j..passwd


Answer: The hex sequence 1f 8b at the beginning of the file represents the magic numbers for the gzip format. The system reads these bytes to identify the file payload as a compressed archive independent of its file extension.

(c) Open file deleted

$ echo ceva > bigfile; exec 3< bigfile; rm bigfile; ls -l /proc/$$/fd; exec 3<&-
lr-x------ 1 codespace codespace 64 Oct 25 21:49 3 -> /home/codespace/os-lab5/bigfile (deleted)


Answer: When bigfile was removed while file descriptor 3 held an open reference (exec 3< bigfile), rm removed its directory link, but the filesystem retained the underlying data blocks. The entry is marked as (deleted) inside /proc/$$/fd. Disk blocks are reclaimed only after closing the descriptor (exec 3<&-).