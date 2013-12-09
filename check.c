#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <gmp.h>
#define MAXSTRING 120000

int count = 0;

int main(int argc, char** argv)
{
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
				char line[MAXSTRING];
				while(fgets(line, MAXSTRING, in)!=NULL)
				{	mpz_set_str(p, line, n);
					if(mpz_probab_prime_p(p, 1)==0)
						gmp_printf("%s (base %d) not prime!\n", line, n);
				}
				fclose(in);
			}
		}
		(void)closedir(dp);
	}
	else
		perror("Couldn't open the directory");

	mpz_clear(p);

	return 0;
}
