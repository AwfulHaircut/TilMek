#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define BUFSIZE 1
#define INPUT_BUFFER_SIZE 64
#define ARGS_BUFFER_SIZE 64

unsigned char tbuf[BUFSIZE];
unsigned char rbuf[BUFSIZE];
size_t nbytes;
struct termios options;

char * path = "/dev/ttyS0";

int fd;

int initiate()
{
	fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);

	tcgetattr(fd, &options);

	options.c_cflag |= (CLOCAL | CREAD);

	cfsetispeed(&options, B2400);
	cfsetospeed(&options, B2400);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CRTSCTS;

	tcsetattr(fd, TCSANOW, &options);

	return 0;
}

int receive()
{

	ssize_t bytes_read = read(fd, rbuf, BUFSIZE);
	printf("%u\n", *rbuf);
	return bytes_read;
}

int transmit(char * data)
{
	strncpy(tbuf, data, BUFSIZE);
	ssize_t bytes_written = write(fd, tbuf, BUFSIZE);
	return bytes_written;
}

char **parse(char *input)
{
	int bufsize = ARGS_BUFFER_SIZE;
	int position = 0;
	char *copy;
	char *arg;
	const char delim[] = " \n";

	copy = strdup(input);

	arg = strsep(&copy,delim);

	if(strcmp(arg,"send") == 0){
		arg = strsep(&copy,delim);
		if(arg != NULL){
			transmit(arg);
		}else{
			printf("Invalid command");
		}
	}else if(strcmp(arg,"r") == 0){
		receive();
	}else if(strcmp(arg,"get") == 0){
		unsigned char get = 255;
		transmit(&get);
		sleep(1);
		receive();
	}else if(strcmp(arg,"set") == 0){
		arg = strsep(&copy,delim);

		char *ptr;

		unsigned char speed = (unsigned char) strtol(arg, &ptr, 10);
		transmit(&speed);
	}else{
		printf("Invalid command\n");
	}

}

char *getInput(void)
{
	char *line = NULL;
	ssize_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}

int main()
{
	char *input;
	char **args;
	ssize_t status;
	initiate();

	do{
		status = 1;
		printf("> ");
		input = getInput();
		//printf("%s",input);
		args = parse(input);

		free(input);
	}while(status);
	close(fd);
	return 0;
}
