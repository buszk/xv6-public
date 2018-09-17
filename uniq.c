#include "types.h"
#include "stat.h"
#include "user.h"

#define BUFSIZE 3

struct linebuffer {
	uint size;		// Allocated
	uint length;	// used
	char *buffer;
};

char buf[BUFSIZE];
struct linebuffer prev_line;
struct linebuffer line;

// C str functions
char* strncpy(char*s, char*t, int n) {
	char *os;
	
	os = s;
	if (n <= 0)
		return os;
	while(n-- > 0 && (*s++ = *t++) != 0)
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

// linebuffer function
void init_linebuffer (struct linebuffer *linebuffer) {
	linebuffer->size = BUFSIZE;
	linebuffer->length = 0;
	linebuffer->buffer = malloc(BUFSIZE);
}

void linebuffer_print(struct linebuffer *linebuffer) {
	//printf(1, linebuffer->buffer, linebuffer->length);
	int i;
	//write(1, "---", 3);
	for (i =  0; i < linebuffer->length; i++ ) write(1, linebuffer->buffer+i, 1);
	//write(1, "\n", 1);
}

void double_linebuffer (struct linebuffer *linebuffer) {
	char *tmp = linebuffer->buffer;
	linebuffer->buffer = malloc(linebuffer->size*2);
	linebuffer->size = linebuffer->size * 2;
	memset(linebuffer->buffer, 0, linebuffer->size);
	strncpy(linebuffer->buffer, tmp, linebuffer->length);
	free((void*) tmp);
}

void append_to_linebuffer(struct linebuffer *linebuffer, char* str, int len) {
	// Make sure enough space
	while(linebuffer->size <=linebuffer->length + len)  {
		double_linebuffer(linebuffer);
	}
	strncpy(linebuffer->buffer + linebuffer->length, str, len);
	linebuffer->length += len;
}

int linebuffer_cmp(struct linebuffer *linebuffer1, struct linebuffer *linebuffer2 ) {
	if (linebuffer1->length != linebuffer2->length) 
		return linebuffer1->length - linebuffer2->length;
	return strncmp(linebuffer1->buffer, linebuffer2->buffer, linebuffer1->length);
}



void uniq(int fd) {
	int i, n;
	int line_start; //tracks the current line
	int line_size;
	int line_ind;
	int first_line = 1;
	line_start = line_size = line_ind = 0;
	// Init linebuffers
	init_linebuffer(&prev_line);
	init_linebuffer(&line);

	while((n = read(fd, buf, sizeof(buf)-1)) > 0) {
		buf[sizeof(buf)-1] = 0;
		//printf(2, "read buf %s\n", buf);
		line_start = 0;
		line_size = 0;
		for (i = 0; i < n; i++ ) {
			if (line_size == 0) {
				line_start = i;
			}
			line_size++;
			if (buf[i] == '\n' || buf[i] == '\r') {// hit new line
				//printf(2, "CurrentLine:%s\n", line.buffer);
				append_to_linebuffer(&line, buf + line_start, i -line_start+1);
				//printf(2, "CurrentLine:%s\n", line.buffer);
				//printf(2, "PrevLine:%s\n", prev_line.buffer);
				if (linebuffer_cmp(&line, &prev_line) != 0 ) {//uniq line
					//printf(2, "Uniq line\n");
					if (first_line == 0) {
						linebuffer_print(&prev_line);
					} else {
						first_line = 0;
					}
					free((void*)prev_line.buffer);
					prev_line.size = line.size;
					prev_line.length = line.length;
					prev_line.buffer = line.buffer;
					init_linebuffer(&line);
				}
				else {
					//printf(2, "Hit same line\n");
					init_linebuffer(&line);
				}
				line_size = 0;
				if (i == n-1) continue;

			}
			if (i == n-1) {// last char of the line, copy remaining buf over to line
				//printf(2, "OldLine:%s\n", line.buffer);
				append_to_linebuffer(&line, buf + line_start, sizeof(buf) - line_start -1);
				//printf(2, "OldLine:%s\n", line.buffer);
			}

		}
		
	}
	//printf(2, "LastLine\n");
	linebuffer_print(&prev_line);
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
