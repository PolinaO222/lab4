#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define BUFSIZE 512

int main(void)
{
	int gen_fd[2], read_fd[2], de_gamma = 0;
	pid_t gen_pid, read_pid;
	char *str;
	char path[BUFSIZE], buf[BUFSIZE], key_path[BUFSIZE];

	memset(path, '\0', BUFSIZE);
	str = "I'll break your file! Enter victim's name: ";
	write(1, str, strlen(str));
	read(0, path, BUFSIZE);
	path[strlen(path)-1] = '\0';
	if (strstr(path, ".gamma") != NULL) {
		memset(key_path, '\0', BUFSIZE);
		strncpy(key_path, path, strlen(path)-5);
		strcat(key_path, "key");
		de_gamma = 1;
		if (pipe(gen_fd) == -1) {
			str = "Oops. No pipe.\n";
			write(1, str, strlen(str));
			exit(1);
		}
		gen_pid = fork();
		if (gen_pid == -1) {
			str = "Oops. No fork.\n";
			write(1, str, strlen(str));
			exit(1);
		} else if (gen_pid == 0) {
			close(gen_fd[0]);
			if (dup2(gen_fd[1], 1) == -1) {
				str = "Oops. No dup.\n";
				write(1, str, strlen(str));
				exit(1);
			}
			close(gen_fd[1]);
			execlp("cat", "cat", key_path, NULL);
		}
	} else {
		de_gamma = 0;
		if (pipe(gen_fd) == -1) {
			str = "Oops. No pipe.\n";
			write(1, str, strlen(str));
			exit(1);
		}
		gen_pid = fork();
		if (gen_pid == -1) {
			str = "Oops. No fork.\n";
			write(1, str, strlen(str));
			exit(1);
		} else if (gen_pid == 0) {
			memset(buf, '\0', BUFSIZE);
			sprintf(buf, "%d", gen_fd[1]);
			execlp("./generator", "./generator", buf, NULL);
		}
	}

	if (pipe(read_fd) == -1) {
		str = "Oops. No pipe.\n";
		write(1, str, strlen(str));
		exit(1);
	}
	read_pid = fork();
	if (read_pid == -1) {
		str = "Oops. No fork.\n";
		write(1, str, strlen(str));
		exit(1);
	} else if (read_pid == 0) {
		close(read_fd[0]);
		if (dup2(read_fd[1], 1) == -1) {
			str = "Oops. No dup.\n";
			write(1, str, strlen(str));
			exit(1);
		}
		close(read_fd[1]);
		execlp("cat", "cat", path, NULL);
	} else {
		close(read_fd[1]);
		sleep(1);

		char xor[BUFSIZE];
		int i, read_cnt, gen_cnt, file, key;

		memset(xor, '\0', BUFSIZE);
		memset(buf, '\0', BUFSIZE);
		strcpy(buf, path);
		strcat(buf, ".gamma");
		file = open(buf, O_RDWR|O_CREAT, 0600);
		if (file == -1) {
			str = "Oops. No file to write to.\n";
			write(1, str, strlen(str));
			exit(1);
		}
		if (de_gamma == 0) {
			strcpy(buf, path);
			strcat(buf, ".key");
			key = open(buf, O_RDWR|O_CREAT, 0600);
			if (key == -1) {
				str = "Oops. No file to save key.\n";
				write(1, str, strlen(str));
				exit(1);
			}
		}
		memset(buf, '\0', BUFSIZE);

		while ((read_cnt = read(read_fd[0], buf, BUFSIZE)) > 0) {
			gen_cnt = read(gen_fd[0], xor, read_cnt);
			if (read_cnt != gen_cnt) {
				str = "Oops.\n";
				write(1, str, strlen(str));
				exit(1);
			}
			for (i = 0; i < read_cnt; i++)
				buf[i] = buf[i] ^ xor[i];
			write(file, buf, read_cnt);
			if (de_gamma == 0)
				write(key, xor, read_cnt);
			memset(buf, '\0', BUFSIZE);
			memset(xor, '\0', BUFSIZE);
		}

		close(read_fd[0]);
		close(file);
		close(key);
		kill(gen_pid, SIGINT);
	}
}
