#include "types.h"
#include "stat.h"
#include "user.h"

#define BUFSIZE 32

uint num = 10;
int i;

struct linebuffer {
	uint size;		// Allocated
	uint length;	// used
	char *buffer;
};

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
	if(linebuffer->buffer)
		write(1, linebuffer->buffer, linebuffer->length);
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

struct linebuffer* access(struct linebuffer** list, int index) {
	return *(list + index);
}

void tail(int fd) {
	// Init linebuffers
	char tmp[64];
	struct linebuffer** buffer_list = malloc(num*sizeof(struct linebuffer *));
	int next = 0;

	struct linebuffer*  line;

	line = malloc(sizeof (struct linebuffer));
	init_linebuffer(line);

	while(read (fd, tmp, sizeof(tmp))) {
		int start = 0;
		i = 0;
		while(i < sizeof(tmp)) {
			if (tmp[i] == '\n') {
				append_to_linebuffer(line, tmp+start, i - start);
				if (access(buffer_list,next%num))
					free_linebuffer(access(buffer_list,next%num));
				*(buffer_list + next%num) = line; // 
				next ++;
				start = i+1;
				line = malloc(sizeof(struct linebuffer));
				init_linebuffer(line);
			}
			i++;
		}
		append_to_linebuffer(line, tmp+start, sizeof(tmp) - start);
	}
	i = 0;
	while (i < num) {
		if (access(buffer_list,(i+next)%num)) {
			print_linebuffer(access(buffer_list, (i+next)%num));
			write(1, "\n", 1);
		}
		i++;
	}
	i = 0;	
	while (i < num) {
		if (access(buffer_list,i)) 
			free_linebuffer(access(buffer_list,i));
		i++;
	}

}

void
usage() {
	printf(2, "Usage: uniq [-NUM] [INPUT]\n");
}

int
stoi(char* s){
	char* i = s;
	int n = 0;
	while(*i<='9' && *i>='0') {
		n *= 10;
		n += (*i-'0');
		i ++;
	}
	return n;
}

int main(int argc, char *argv[]) {
	int fd = 0;
	int ind;
	for (ind = 1; ind < argc; ind++) {
		if (strlen(argv[ind]) >= 2 && argv[ind][0] == '-') {
			num = stoi(argv[ind]+1);
		}
		else {
			fd = open(argv[ind], 0);
		}
	}
	tail(fd);
	if (fd) {
		close(fd);
	}
	exit();
}
