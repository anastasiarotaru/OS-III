#!/usr/bin/env bash
# Lab 5 - The File System with Your Own Hands (Linux)
# Rulează toate experimentele din laborator și salvează output-ul în output/full_log.txt.
# Nu rula ca root: experimentele cu permisiuni nu ar fi corecte.

set -u

if [ "$(id -u)" -eq 0 ]; then
    echo "EROARE: nu rula scriptul ca root. Folosește utilizatorul normal." >&2
    exit 1
fi

WORKDIR="$HOME/os-lab5"
REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
LOG="$REPO_DIR/output/full_log.txt"

mkdir -p "$WORKDIR" "$REPO_DIR/output"
cd "$WORKDIR" || exit 1

# Curăță rezultatele din rulările anterioare (doar în folderul de lucru)
rm -rf notes.txt hard.txt soft.txt d one.txt big.img s.txt new.txt newdir box notes2.txt bigfile 2>/dev/null
chmod -R u+rwx . 2>/dev/null

# r "comandă" : afișează comanda, o execută cu bash și afișează output-ul
r() {
    echo "\$ $1"
    bash -c "$1" 2>&1
    echo
}

section() {
    echo "=============================================================="
    echo "$1"
    echo "=============================================================="
    echo
}

{
echo "Lab 5 - rulare: $(date)"
echo "Utilizator: $(whoami)   Director: $(pwd)"
echo "Sistem: $(uname -a)"
echo

section "PARTEA 1 - Arborele și tipurile de fișiere"
r 'ls /'
r 'ls -ld /etc /etc/passwd /bin /dev/null /tmp'
r 'ls -l /dev | head'
r 'file /usr/bin/ls /etc/passwd /bin'
r 'du -sh /var/log 2>/dev/null ; du -sh /usr 2>/dev/null'

section "PARTEA 2.1 - Inode și nume"
r 'echo hello > notes.txt'
r 'stat notes.txt'
r 'ls -li notes.txt'

section "PARTEA 2.2 - Hard link"
r 'ln notes.txt hard.txt'
r 'ls -li notes.txt hard.txt'
r 'rm notes.txt'
r 'cat hard.txt ; ls -li hard.txt'

section "PARTEA 2.3 - Symbolic link"
r 'ln -s hard.txt soft.txt'
r 'ls -li hard.txt soft.txt'
r 'cat soft.txt'
r 'rm hard.txt'
r 'cat soft.txt ; ls -l soft.txt'

section "PARTEA 2.4 - Link-urile directoarelor"
r 'mkdir d ; ls -ld d'
r 'mkdir d/x d/y ; ls -ld d'

section "PARTEA 2.5 - Dimensiunea nu înseamnă spațiu"
r 'echo x > one.txt ; ls -l one.txt ; du -h one.txt'
r 'truncate -s 1G big.img ; ls -lh big.img ; du -h big.img'
r 'rm big.img'

section "PARTEA 3.1 - Permisiuni pe fișiere"
r 'echo secret > s.txt'
r 'chmod 600 s.txt ; ls -l s.txt'
r 'chmod 000 s.txt ; cat s.txt'
r 'chmod u+r s.txt ; cat s.txt'
r 'umask ; touch new.txt ; mkdir newdir ; ls -ld new.txt newdir'

section "PARTEA 3.2 - Permisiuni pe directoare"
r 'mkdir box ; echo hi > box/f'
r 'chmod u=r,go= box ; ls box ; cat box/f'
r 'chmod u=x,go= box ; ls box ; cat box/f'
r 'chmod 755 box'

section "PARTEA 3.3 - Biți speciali și fișiere ale altora"
r 'ls -l /usr/bin/passwd ; ls -ld /tmp'
r 'cat /etc/shadow'
r 'ls -l /etc/shadow'

section "PARTEA 4 - Discuri și kernelul ca fișiere"
r 'lsblk'
r 'df -h'
r 'df -h /'
r 'lsblk -f'
r 'mount | grep " / "'
r 'grep -c processor /proc/cpuinfo'
r 'head -3 /proc/meminfo'
r 'head -5 /proc/self/status'
r 'ls -l /proc/self/fd'

section "BONUS (a) - Magic numbers"
r 'gzip -c /etc/passwd > notes2.txt ; file notes2.txt ; xxd notes2.txt | head -1'

section "BONUS (c) - Fișier deschis și șters"
r 'echo ceva > bigfile; exec 3< bigfile; rm bigfile; ls -l /proc/$$/fd; exec 3<&-'

echo "Gata. Rulare terminată la $(date)."
} 2>&1 | tee "$LOG"

echo
echo "Output salvat în: $LOG"
echo "Copiază rezultatele relevante în report.md."
