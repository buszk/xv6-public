#include "types.h"
#include "stat.h"
#include "user.h"

char line[512];
char prev_line[512];
char buf[512];

char* strncpy(char*s, char*t, int n) {
	char *os;
	
	os = s;
	if (n <= 0)
		return os;
	while(--n > 0 && (*s++ = *t++) != 0)
		;
	return os;
}

char* safestrcpy(char* s, char* t, int n) {
	char *os;

	os = s;
	if (n <= 0)
		return os;
	while(--n > 0 && (*s++ = *t++) != 0)
		;
	*s = 0;
	return os;
}

int strncmp(const char*p, const char*q, uint n) {
	while(n > 0 && *p && *p == *q)
		n--, p++, q++;
	if (n== 0)
		return 0;
	return (uchar)*p - (uchar)*q;
}

void uniq(int fd) {
	int n, i; //n: count of read input, i: for loop counter
	int line_start; //tracks the current line
	int line_size;
	int line_ind;
	int first_line = 1;
	line_start = line_size = line_ind = 0;
	while((n = read(fd, buf, sizeof(buf))) > 0) {
		line_start = 0;
		for (i = 0; i < n; i++ ) {
			if (line_size == 0) {
				line_start = i;
			}
			line_size++;
			if (buf[i] == '\n' || buf[i] == '\r') {// hit new line
				safestrcpy(line + line_ind, buf + line_start, ++line_size); //copy to line buffer
				line_ind = 0;
				if (strncmp(line, prev_line, line_size) != 0 ) {//uniq line
					if (first_line == 0) {
						printf(1, prev_line, sizeof(prev_line));
					} else {
						first_line = 0;
					}
					safestrcpy(prev_line, line, line_size); // copy to prev line buffer
					line_size = 0;
				}
				else {
					printf(2, "Hit same line\n", 15);
				}

			}
			if (i == n-1) {// last char of the line, copy remaining buf over to line
				strncpy(line, buf + line_start, line_size);
				line_ind = line_size;
			}

		}
		
	}
	printf(1, line, sizeof(line));
}


int
main(int argc, char *argv[]) {
	int fd = 0;
	if (argc == 2) {
		fd = open(argv[1], 0);
		uniq(fd);
		close(fd);
	}
	else {
		uniq(0);
		close(0);
	}
	exit();
}
