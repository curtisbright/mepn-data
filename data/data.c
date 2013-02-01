#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#define MAXSTRING 50000

struct data {
	int level;
	int size;
	int width;
	int unsolved;
};

struct data* levels = NULL;
int count = 0;

int compare(const void* a, const void* b)
{	return ((struct data*)a)->level - ((struct data*)b)->level;
}

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
			{	count++;
				strcpy(strchr(filename+7, '.'), "\0");
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

				//printf("%d\t size %d\tlength %d\n", n, numlines, maxsize);
				levels = realloc(levels, sizeof(struct data)*count);
				levels[count-1].level = n;
				levels[count-1].size = numlines;
				levels[count-1].width = maxsize;

				fclose(in);
				for(int i=0; i<numlines; i++)
					free(lines[i]);
				free(lines);

				numlines = 0;
				sprintf(filename, "unsolved.%d.txt", n);
				in = fopen(filename, "r");
				while(fgets(line,MAXSTRING,in)!=NULL)
					numlines++;
				fclose(in);
				levels[count-1].unsolved = numlines;
			}
		}
		(void)closedir(dp);
	}
	else
		perror("Couldn't open the directory");

	qsort(levels, count, sizeof(struct data), compare);

	printf("level\tsize\twidth\tunsolved\n");
	for(int i=0; i<count; i++)
		printf("%d\t%d\t%d\t%d\n", levels[i].level, levels[i].size, levels[i].width, levels[i].unsolved);

	return 0;
}
