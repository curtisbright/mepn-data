#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#define MAXSTRING 50000

int main(int argc, char** argv)
{
	DIR *dp;
	struct dirent *ep;

	dp = opendir("./");
	if(dp != NULL)
	{	while(ep = readdir(dp))
		{	char filename[100];
			strcpy(filename, ep->d_name);
			filename[6] = '\0';
			if(strcmp(filename, "kernel")==0)
			{	strcpy(strchr(filename+7, '.'), "\0");
				int n = atoi(filename+7);
				FILE* in = fopen(ep->d_name, "r");

				char** lines = NULL;
				int numlines = 0;
				int maxsize = 0;

				char line[MAXSTRING];
				while(fgets(line, MAXSTRING, in)!=NULL)
				{	numlines++;
					lines = realloc(lines, sizeof(char*)*numlines);
					lines[numlines-1] = malloc(MAXSTRING);
					strcpy(lines[numlines-1], line);
					lines[numlines-1][strlen(line)-1] = '\0';
					if(strlen(lines[numlines-1])>maxsize)
						maxsize = strlen(lines[numlines-1]);
				}

				printf("%d\t size %d\tlength %d\n", n, numlines, maxsize);

				fclose(in);
				for(int i=0; i<numlines; i++)
					free(lines[i]);
				free(lines);
			}
		}
		(void)closedir(dp);
	}
	else
		perror ("Couldn't open the directory");

	return 0;
}
