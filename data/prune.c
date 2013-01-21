#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#define MAXSTRING 50000

int subword(char* prime, char* start, char middle, char* end, int* k)
{	int i=0, j=0, l=0;
	*k = 0;
	while(1)
	{	if(j!=strlen(start))
		{	if(prime[i]==start[j])
				i++;
			j++;
		}
		else if(l==0 && prime[i]==middle)
		{	(*k)++;
			i++;
		}
		else
		{	if(prime[i]==end[l])
				i++;
			l++;
		}
		if(i==strlen(prime))
			return 1;
		else if(j==strlen(start) && l==strlen(end))
			return 0;
	}
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
				strcpy(filename, "tmp-");
				strcat(filename, ep->d_name);

				printf("pruning base %d...\n", n);

				char** lines = NULL;
				int numlines = 0;

				char line[MAXSTRING];
				while(fgets(line, MAXSTRING, in)!=NULL)
				{	numlines++;
					lines = realloc(lines, sizeof(char*)*numlines);
					lines[numlines-1] = malloc(MAXSTRING);
					strcpy(lines[numlines-1], line);
					lines[numlines-1][strlen(line)-1] = '\0';
				}

				fclose(in);

				FILE* out = fopen(filename, "w");

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

				remove(ep->d_name);
				rename(filename, ep->d_name);
			}
		}
		(void)closedir(dp);
	}
	else
		perror ("Couldn't open the directory");

	return 0;
}
