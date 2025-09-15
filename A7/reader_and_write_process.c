#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/myfifo"
#define MAX_LEN 80

int main(int argc, char * argv[]){
	
	char buff[MAX_LEN];
	int fdp = mkfifo(FIFO_NAME, S_IFIFO | 0666);
	int fifo = open(FIFO_NAME, O_RDONLY);
	printf("Pipe is open for read\n");
	
	for(;;){
		int ret = read(fifo, buff, MAX_LEN);
		printf("Received from parent: %s\n", buff);
	}
	close(fifo);
	return 0;
}
