#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BUFSIZE 512

int main(int argc, char *argv[])
{
	char randnum[BUFSIZE];
	char *str, **c = NULL;
	int i, pipefd = -1;

	srand(time(NULL));
	pipefd = (int)strtod(argv[1], c);
	if (c != NULL) {
		str = "Oops. No pipe.\n";
		write(1, str, strlen(str));
		exit(1);
	}

	while (1) {
		for (i = 0; i < BUFSIZE; i++)
			randnum[i] = (char)(rand() % 256);
		write(pipefd, &randnum, BUFSIZE);
	}
}
