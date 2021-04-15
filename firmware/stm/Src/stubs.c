/*
 * stubs.c
 *
 *  Created on: May 4, 2017
 *      Author: addaon
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#undef errno
extern int errno;

void _exit(int status) {
		for(;;);
}

int _kill(int pid, int sig) {
	errno = EINVAL;
	return -1;
}

int _getpid() {
	return 1;
}

caddr_t _sbrk(int incr) {
	errno = ENOMEM;
	return (caddr_t) -1;
}

int _write(int file, char *ptr, int len) {
	errno = EBADF;
	return -1;
}

int _close(int file) {
	return -1;
}

int _fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file) {
	errno = EBADF;
	return 0;
}

int _lseek(int file, int ptr, int dir) {
	return 0;
}

int _read(int file, char *ptr, int len) {
	errno = EBADF;
	return -1;
}
