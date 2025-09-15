#include <stdio.h>
#include <unistd.h>

#define MAX_CHAR 80
#define PIPE_IN 1
#define PIPE_OUT 0

int main(int argc, char * argv[]){

	int fds[2];
	char buff[MAX_CHAR];
	char pipeBuffOut[MAX_CHAR];
	
	//crea pipe file descriptors
	int ret = pipe(fds);
	
	//lee linea de consola
	char * str = fgets(buff, MAX_CHAR, stdin);
	
	//escribe linea en estrada del pipe
	ret = write(fds[PIPE_IN], buff, MAX_CHAR);
	//lee la salida del pipe
	ret = read(fds[PIPE_OUT], pipeBuffOut, MAX_CHAR);
	
	//imprime la salida del pipe
	printf("%s", pipeBuffOut);
	
	return 0;
}
