#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <string.h>
#include <gmp.h>
#define MAXSTRING 120000

int count = 0;

int main(int argc, char** argv)
{
	struct timeval start, end;
	DIR *dp;
	struct dirent *ep;
	mpz_t p;
	mpz_init(p);

	dp = opendir("./data");
	if(dp != NULL)
	{	while(ep = readdir(dp))
		{	char filename[100];
			strcpy(filename, ep->d_name);
			filename[7] = '\0';
			if(strcmp(filename, "minimal")==0)
			{	strcpy(strchr(filename+8, '.'), "\0");
				int n = atoi(filename+8);
				sprintf(filename, "data/%s", ep->d_name);
				FILE* in = fopen(filename, "r");
				printf("Checking base %d...\n", n);
				int count = 0;
				gettimeofday(&start, 0);
				char line[MAXSTRING];
				while(fgets(line, MAXSTRING, in)!=NULL)
				{	count++;
					mpz_set_str(p, line, n);
					if(mpz_probab_prime_p(p, 1)==0)
						gmp_printf("%s (base %d) not prime!\n", line, n);
				}
				fclose(in);
				gettimeofday(&end, 0);
				double elapsed = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec)/1000000.0F);
				printf("Completed base %d: %d minimal elements checked for pseudoprimality in %.2f sec\n", n, count, elapsed);
			}
		}
		(void)closedir(dp);
	}
	else
		perror("Couldn't open the directory");

	mpz_clear(p);

	return 0;
}
