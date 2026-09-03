/**
 ******************************************************************************
 * @file    syscalls.c
 * @brief   Minimal newlib system-call stubs that send stdio to the console
 *          driver (provided code — do not change it).
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 ******************************************************************************
 */
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>

extern int __io_putchar(int ch);   /* console.c */
extern int __io_getchar(void);     /* console.c */

int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        __io_putchar((int)ptr[i]);
    }
    return len;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        *ptr++ = (char)__io_getchar();
    }
    return len;
}

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}
