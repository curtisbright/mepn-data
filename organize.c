#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#define MAXSTRING 120000

int compare(const void* a, const void* b)
{	const char **ia = (const char **)a;
	const char **ib = (const char **)b;
	if(strlen(*ia) < strlen(*ib))
		return -1;
	else if(strlen(*ia) > strlen(*ib))
		return 1;
	else
		return strcmp(*ia, *ib);
}

int nosubword(char* prime, char* candidate)
{	int k=0;
	for(int i=0; i<strlen(candidate); i++)
	{	if(candidate[i]==prime[k])
			k++;
		if(k==strlen(prime))
			return 0;
	}
	return 1;
}

int main(int argc, char** argv)
{
	DIR *dp;
	struct dirent *ep;

	dp = opendir("./data");
	if(dp != NULL)
	{	while(ep = readdir(dp))
		{	char filename[100], tmpfilename[100];
			strcpy(filename, ep->d_name);
			filename[7] = '\0';
			if(strcmp(filename, "minimal")==0)
			{	strcpy(strchr(filename+8, '.'), "\0");
				int n = atoi(filename+8);
				sprintf(filename, "data/%s", ep->d_name);
				FILE* in = fopen(filename, "r");
				sprintf(tmpfilename, "data/org-%s", ep->d_name);

				printf("organizing base %d...\n", n);

				char** lines = NULL;
				int numlines = 0;

				char line[MAXSTRING];
				while(fgets(line, MAXSTRING, in)!=NULL)
				{	numlines++;
					lines = realloc(lines, sizeof(char*)*numlines);
					lines[numlines-1] = malloc(strlen(line)+1);
					strcpy(lines[numlines-1], line);
					lines[numlines-1][strlen(line)-1] = '\0';
				}

				fclose(in);

				qsort(lines, numlines, sizeof(char*), compare);

				FILE* out = fopen(tmpfilename, "w");

				for(int i=0; i<numlines; i++)
				{	int hassubword = 0;
					for(int j=0; j<i; j++)
					{	hassubword = 1-nosubword(lines[j], lines[i]);
						if(hassubword)
						{	printf("%s has a subword %s\n", lines[i], lines[j]);
							break;
						}
					}
					if(!hassubword)
						fprintf(out, "%s\n", lines[i]);
				}	

				fclose(out);

				for(int i=0; i<numlines; i++)
					free(lines[i]);

				remove(filename);
				rename(tmpfilename, filename);
			}
		}
		(void)closedir(dp);
	}
	else
		perror("Couldn't open the directory");

	return 0;
}
