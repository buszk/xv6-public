#include "types.h"
#include "stat.h"
#include "user.h"

#define BUFSIZE 3

uint count = 0;
uint duplicate = 0;
uint ignore = 0;
int c = 0;

struct linebuffer {
	uint size;		// Allocated
	uint length;	// used
	char *buffer;
};

char buf[BUFSIZE];
struct linebuffer prev_line;	// On stack, keeps track of previous line
struct linebuffer line;			// On stack, keeps track of current line

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

char tolower(const char* c) {
	if (*c >= 65 && *c <= 90) return *c+32;
	else if (*c >=97 && *c <=122) return *c;
	else return *c;
}

/* A simple char compare function with case-sentive option */
int chrcmp(const char* c1, const char*c2, uint cs) {
	if (cs == 1) {return *c1 - *c2; printf(1, "Case-sensitive\n");}
	else if (*c1 == *c2) return 0;
	else return tolower(c1) - tolower(c2);
}
			

int strncmp(const char*p, const char*q, uint n, uint cs) {
	while(n > 0 && *p && chrcmp(p, q, cs) == 0)
		n--, p++, q++;
	if (n== 0)
		return 0;
	return (uchar)*p - (uchar)*q;
}

// A simple print number with controlled space function
void _printnum(int num, int space){
	int max, i;
	for (max = 1; num >= max; space --) max *= 10;
	if (space < 0) {
		printf(2, "Not enought space for printing\n");
		exit();
	}
	else {
		for (i = 0; i < space; i++) write(1, " ", 1);
		printf(1, "%d ", num);
	}
}


// linebuffer function
void init_linebuffer (struct linebuffer *linebuffer) {
	linebuffer->size = BUFSIZE;
	linebuffer->length = 0;
	linebuffer->buffer = malloc(BUFSIZE);
}

void free_linebuffer(struct linebuffer *linebuffer) {
	if (linebuffer->buffer) {
		free((void*)linebuffer->buffer);
	}
}

void print_linebuffer(struct linebuffer *linebuffer) {
	int i;
	if (count) _printnum(c, 4); // print occurrence if with the option
	if (!duplicate || c > 1)
		for (i =  0; i < linebuffer->length; i++ ) write(1, linebuffer->buffer+i, 1);
	c = 1;
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
	// Make sure linebuffer has enough space
	while(linebuffer->size <=linebuffer->length + len)  {
		double_linebuffer(linebuffer);
	}
	strncpy(linebuffer->buffer + linebuffer->length, str, len);
	linebuffer->length += len;
}

int cmp_linebuffer(struct linebuffer *linebuffer1, struct linebuffer *linebuffer2 ) {
	if (linebuffer1->length != linebuffer2->length) 
		return linebuffer1->length - linebuffer2->length;
	return strncmp(linebuffer1->buffer, linebuffer2->buffer, linebuffer1->length, (ignore==0));
}



void uniq(int fd) {
	int i, n;
	int line_start; //tracks the current line
	int line_size;
	int first_line = 1; //if the nextline is the first line
	line_start = line_size = 0;
	c = 1;
	// Init linebuffers
	init_linebuffer(&prev_line);
	init_linebuffer(&line);

	while((n = read(fd, buf, sizeof(buf)-1)) > 0) {
		buf[sizeof(buf)-1] = 0;
		line_start = 0;
		line_size = 0;
		for (i = 0; i < n; i++ ) {
			if (line_size == 0) {
				line_start = i;
			}
			line_size++;
			if (buf[i] == '\n' || buf[i] == '\r') {// hit new line
				append_to_linebuffer(&line, buf + line_start, i -line_start+1);
				if (cmp_linebuffer(&line, &prev_line) != 0 ) {//uniq line
					if (first_line == 0) {
						print_linebuffer(&prev_line); //Postpone print
					} else {
						first_line = 0;
					}
					free_linebuffer(&prev_line);
					prev_line = line; // shollow copy
					init_linebuffer(&line); // reinit current linebuffer
				}
				else { // Line is the same as prev_line
					free_linebuffer(&line);
					init_linebuffer(&line);
					c++;
				}
				line_size = 0;
				if (i == n-1) continue;

			}
			if (i == n-1) {// last char of the line, copy remaining buf over to line
				append_to_linebuffer(&line, buf + line_start, sizeof(buf) - line_start -1);
			}

		}
		
	}
	// The end. Just print whatever in prev_line
	print_linebuffer(&prev_line);
}

void
usage() {
	printf(2, "Usage: uniq [-c|-d|-i] [INPUT]\n");
}

int
main(int argc, char *argv[]) {
	int fd = 0;
	int ind;
	for (ind = 1; ind < argc; ind++) {
		if (strlen(argv[ind]) >= 2 && argv[ind][0] == '-') {
			switch(argv[ind][1]){
			case 'c':
				count = 1;
				break;
			case 'd':
				duplicate = 1;
				break;
			case 'i':
				ignore = 1;
				break;
			default:
				printf(2, "Unknow option\n");
				usage();
				exit();
				break;
			}
		}
		else {
			fd = open(argv[ind], 0);
			break;
		}
	}
	uniq(fd);
	if (fd) {
		close(fd);
	}
	exit();
}
