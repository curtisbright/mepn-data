#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gmp.h>

int main(int argc, char** argv)
{
	DIR *dp;
	struct dirent *ep;

	if(argc==1)
	{	printf("After sieving has been done, this program\n");
		printf("converts the sieve data into ABC format\n");
		printf("which is suitable for testing with LLR\n");
		return 0;
	}

	FILE* abc = fopen("data/sieve.abc", "w");

	printf("ABC ($a*$b^$c$d)/$e\n");
	fprintf(abc, "ABC ($a*$b^$c$d)/$e\n");

	for(int i=atoi(argv[1]); i<60000; i++)
	{	dp = opendir("./data");
		int count = 0;
		if(dp != NULL)
		{	while(ep = readdir(dp))
			{	char filename[100];
				char infilename[100];
				strcpy(filename, ep->d_name);
				filename[8] = '\0';
				if(strcmp(filename, "unsolved")==0)
				{	strcpy(strchr(filename+9, '.'), "\0");
					int base = atoi(filename+9);
					sprintf(infilename, "data/%s", ep->d_name);
					FILE* in = fopen(infilename, "r");
					char line[100];
					char start[100];
					char middle[2];
					char end[100];
					while(fgets(line, 100, in)!=NULL)
					{	int l = (int)(strchr(line, '*')-line);
						middle[0] = line[l-1];
						middle[1] = '\0';
						line[strlen(line)-1] = '\0';
						line[l-1] = '\0';
						strcpy(start, line);
						strcpy(end, line+l+1);

						int zlen = strlen(end);
						mpz_t x, y, z, temp, temp2, temp3, temp10;
						mpz_inits(x, y, z, temp, temp2, temp3, temp10, NULL);
						mpz_set_str(x, start, base);
						mpz_set_str(y, middle, base);
						mpz_set_str(z, end, base);
						mpz_gcd_ui(temp10, y, base-1);
						int g = mpz_get_ui(temp10);
						mpz_divexact_ui(temp, y, g);
						mpz_set(temp2, temp);
						mpz_addmul_ui(temp, x, (base-1)/g);
						mpz_ui_pow_ui(temp3, base, zlen);
						mpz_mul(temp, temp, temp3);
						mpz_mul(temp2, temp2, temp3);
						mpz_submul_ui(temp2, z, (base-1)/g);
						mpz_neg(temp3, temp2);

						char family[100];
						gmp_sprintf(family, "%Zd*%d^n%+Zd\n", temp, base, temp3);

						// Find an exponent
						int num = -1;
						char sievefilename[100], sievetmpfilename[100];
						sprintf(sievefilename, "data/sieve.%d.txt", base);
						FILE* sieve = fopen(sievefilename, "r");
						if(sieve!=NULL)
						{	while(fgets(line, 100, sieve)!=NULL)
							{	if(strcmp(line, family)==0)
								{	if(fgets(line, 100, sieve)!=NULL && strchr(line, '*')==NULL)
										num = atoi(line);
									break;
								}
							}
							fclose(sieve);
						}

						if(num!=i)
							continue;

						gmp_printf("%Zd %d %d %+Zd %d\n", temp, base, num, temp3, (base-1)/g);
						gmp_fprintf(abc, "%Zd %d %d %+Zd %d\n", temp, base, num, temp3, (base-1)/g);

						// Remove the exponent from the sieve file
						sieve = fopen(sievefilename, "r");
						sprintf(sievetmpfilename, "data/tmp-sieve.%d.txt", base);
						FILE* sieveout = fopen(sievetmpfilename, "w");
						char sieveline[100];
						int thisfamily = 0;
						while(fgets(sieveline, 100, sieve)!=NULL)
						{	if(strchr(sieveline, '*')!=NULL)
								thisfamily = 0;
							if(strcmp(sieveline, family)==0)
								thisfamily = 1;
							if(!(thisfamily==1 && strcmp(line, sieveline)==0))
								fprintf(sieveout, "%s", sieveline);
						}
						fclose(sieve);
						fclose(sieveout);
						remove(sievefilename);
						rename(sievetmpfilename, sievefilename);
					}
					fclose(in);
				}
			}
			(void)closedir(dp);
		}
		else
			perror("Couldn't open the directory");
	}

	fclose(abc);

	return 0;
}
