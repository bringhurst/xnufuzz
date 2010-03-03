/*
 * Use this program at your own risk. It will cause data loss and crashes.
 */
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sched.h>

/*
 * This ignore list is customized for xnu-1486.2.11 (10.6.2).
 */
int ignore[] = {
    8, /* old creat */
    11, /* old execv */
    17, /* old break */
    19, /* old lseek */
    21, /* old mount */
    22, /* old umount */
    38, /* old stat */
    40, /* old lstat */
    45, /* old ktrace */
    62, /* old fstat */
    63, /* used internally , reserved */
    64, /* old getpagesize */
    67, /* old vread */
    68, /* old vwrite */
    69, /* old sbrk */
    70, /* old sstk */
    71, /* old mmap */
    72, /* old vadvise */
    77, /* old vlimit */
    84, /* old wait */
    87, /* old gethostname */
    88, /* old sethostname */
    91, /* old getdopt */
    94, /* old setdopt */
    99, /* old accept */
    101, /* old send */
    102, /* old recv */
    103, /* old sigreturn */
    107, /* old vtimes */
    108, /* old sigvec */
    109, /* old sigblock */
    110, /* old sigsetmask */
    112, /* old sigstack */
    113, /* old recvmsg */
    114, /* old sendmsg */
    115, /* old vtrace */
    119, /* old resuba */
    125, /* old recvfrom */
    129, /* old truncate */
    130, /* old ftruncate */
    141, /* old getpeername */
    143, /* old sethostid */
    144, /* old getrlimit */
    145, /* old setrlimit */
    146, /* old killpg */
    148, /* old setquota */
    149, /* old qquota */
    150, /* old getsockname */
    156, /* old getdirentries */
    160, /* old async_daemon */
    162, /* old getdomainname */
    163, /* old setdomainname */
    164, /* */
    166, /* old exportfs */
    168, /* old ustat */
    170, /* old table */
    171, /* old wait3 */
    172, /* old rpause */
    174, /* old getdents */
    175, /* old gc_control */
    177, /* */
    178, /* */
    179, /* */
    186, /* */
    193, /* */
    198, /* __syscall */
    213, /* Reserved for AppleTalk */
    214, /* */
    215, /* */
    224, /* old checkuseraccess / fsgetpath ( which moved to 427 ) */
    246, /* */
    249, /* */
    257, /* */
    312, /* old __pthread_cond_timedwait */
    321, /* old __pthread_cond_wait */
    323, /* */
    326, /* */
    335, /* old utrace */
    352, /* */
    373, /* */
    374, /* */
    375, /* */
    376, /* */
    377, /* */
    378, /* */
    379
};

int getseed(void) {
    int fd = open("/dev/urandom", O_RDONLY);
    int r;

    if (fd < 0) {
        perror("open");
        exit(0);
    }

    read(fd, &r, sizeof(r));
    close(fd);

    return(r);
}

void getrand(char *p) {
    int fd = open("/dev/urandom", O_RDONLY);

    if (fd < 0) {
        perror("open");
        exit(0);
    }

    read(fd, p, 128);
    close(fd);

    return;
}

unsigned long getnumber(void) {
    int state;
    state = rand() % 5;

    switch(state) {
        case 0:
            return rand();
            break;
        case 1:
            return( 0xffffff00 | (rand() % 256));
        case 2:
            return 0x8000;
        case 3:
            return 0xffff;
        case 4:
            return 0x80000000;
    }
}

unsigned long getarg(void) {
    int state = rand() % 5;
    static char b[128];

    switch(state) {
        /* userland addr */
        case 0:
            return 0x0804fd00;
            break;
        /* unmapped addr */
        case 1: return 0x0000a000;
            break;
        /* kernel addr, this is a guess ... should actually get a real one ...  */
        case 2: return 0xc01fa0b6;
            break;
        /* some number */
        case 3: return getnumber();
            break;
        case 4: getrand(b);
            return &b;
            break;
    }
}

int mem_fuzz() {
    unsigned long arg[8];
    long *p;
    int i, syscallnr, flag;
    srand(getseed());

    while(1) {
        do {
            flag = 0;
            syscallnr = (rand() % 253);

            for (i = 0; i < (sizeof(ignore) / sizeof(ignore[0])); i++) {
                if (ignore[i] == syscallnr ) {
                    flag = 1;
                    break;
                }
            }
        } while (flag);

        p = arg;

        for (i = 0; i < 8; i++) {
            *p++ = getarg();
        }

        printf("syscall(%d, %p, %p, %p, %p, %p, %p, %p, %p);\n",
        syscallnr, arg[0], arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7]);
        fflush(stdout);
        usleep(5);
        syscall(syscallnr, arg[0], arg[1],arg[2],arg[3],arg[4],arg[5],arg[6],arg[7]);
    }
}

int main(void) {
    mem_fuzz();
}

/* EOF */
