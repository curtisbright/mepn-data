#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <gmp.h>
#define MAXSTRING 50000

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

	mpz_t p;
	mpz_init(p);

	dp = opendir("./");
	if(dp != NULL)
	{	while(ep = readdir(dp))
		{	char filename[100];
			strcpy(filename, ep->d_name);
			filename[6] = '\0';
			if(strcmp(filename, "kernel")==0)
			{	strcpy(strchr(filename+7, '.'), "\0");
				int n = atoi(filename+7);

				if(n==atoi(argv[1]))
				{	FILE* in = fopen(ep->d_name, "r");
					//printf("outputting base %d...\n", n);

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

					qsort(lines, numlines, sizeof(char*), compare);

					printf("[");
					int first = 1;
					for(int i=0; i<numlines; i++)
					{	mpz_set_str(p, lines[i], n);
						if(!first)
							printf(",\n");
						first = 0;
						gmp_printf("%s", lines[i], p);
					}
					printf("]\n");
				}
			}
		}
		(void)closedir(dp);
	}
	else
		perror ("Couldn't open the directory");

	mpz_clear(p);

	return 0;
}
