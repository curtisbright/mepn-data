#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int pid = -1;

void sig_handler(int signo)
{	if(signo == SIGINT)
	{	if(pid!=-1)
			kill(pid, SIGINT);
		system("./merge");
		exit(0);
	}
}

int main(int argc, char** argv)
{	if(argc==1)
	{	printf("Given a base on the command line, this program runs\n");
		printf("srsieve on that base sieve file and then\n");
		printf("merges the results with the search data\n");
	}
	else if(argc>1)
	{	signal(SIGINT, sig_handler);
		for(;;)
		{	pid = fork();
			if(pid==0)
			{	char filename[25];
				sprintf(filename, "srsieve/sieve.%s.txt", argv[1]);
				execlp("timeout", "timeout", "1m", "srsieve/srsieve", filename, "-o", filename, NULL);
			}
			int status;
			wait(&status);
			system("./merge");
		}
	}

	return 0;
}
