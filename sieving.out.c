#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gmp.h>
#define MAXSTRING 60100

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
		else if(l==strlen(end)+1)
			return 0;
	}
}

int main(int argc, char** argv)
{
	DIR *dp;
	struct dirent *ep;     

	int result;
	clock_t begin, end;	
	double time_spent;

	mpz_t p;
	mpz_init(p);

	for(int i=0; i<60000; i++)
	{	dp = opendir("./data");
		int count=0;
		if(dp != NULL)
		{	while(ep = readdir(dp))
			{	char filename[100];
				char infilename[100];
				char outfilename[100];
				strcpy(filename, ep->d_name);
				filename[8] = '\0';
				if(strcmp(filename, "unsolved")==0)
				{	strcpy(strchr(filename+9, '.'), "\0");
					int base = atoi(filename+9);
					sprintf(infilename, "data/%s", ep->d_name);
					FILE* in = fopen(infilename, "r");
					sprintf(outfilename, "data/tmp-%s", ep->d_name);
					FILE* out = fopen(outfilename, "w");
					char line[100];
					char start[100];
					char middle[2];
					char end[100];
					char candidate[MAXSTRING];
					while(fgets(line, 100, in)!=NULL)
					{	count++;
						int l = (int)(strchr(line, '*')-line);
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
						if(mpz_sgn(temp2)>=0)
							gmp_sprintf(family, "%Zd*%d^n-%Zd\n", temp, base, temp2);
						else
							gmp_sprintf(family, "%Zd*%d^n+%Zd\n", temp, base, temp3);

						int num = -1;
						char sieveout[100];
						sprintf(sieveout, "data/sieve.%d.out.txt", base);
						FILE* sieve = fopen(sieveout, "r");
						if(sieve!=NULL)
						{	while(fgets(line, 100, sieve)!=NULL)
							{	if(strcmp(line, family)==0)
								{	fgets(line, 100, sieve);
									if(strchr(line, '*')==NULL)
										num	= atoi(line);
									break;
								}
							}
							fclose(sieve);
						}

						if(num==-1)
							continue;

						//printf("base: %d start: %s middle: %c end: %s\n", base, start, middle[0], end);
						strcpy(candidate, start);
						for(int j=0; j<num; j++)
							sprintf(candidate, "%s%c", candidate, middle[0]);
						strcat(candidate, end);
						//printf("candidate: %s\n", candidate);

						char kernelfilename[100];
						sprintf(kernelfilename, "data/minimal.%d.txt", base);

						FILE* kernel = fopen(kernelfilename, "r");
						char prime[MAXSTRING];
						int hassubword = 0;
						//printf("Checking %s%c*%s (base %d)...\n", start, middle, end, n);
						while(fgets(prime, MAXSTRING, kernel)!=NULL)
						{	prime[strlen(prime)-1] = '\0';
							int k;
							if(subword(prime, start, middle[0], end, &k)==1)
							{	if(k<=num)
								{	hassubword = 1;
									break;
								}
							}
						}
						fclose(kernel);

						if(hassubword)
						{	printf("%s%c^(%d)%s (base %d) has a kernel subword %s\n", start, middle[0], num, end, base, prime);

							char sieveoutw[100];
							sieve = fopen(sieveout, "r");
							sprintf(sieveoutw, "data/tmp-sieve.%d.out.txt", base);
							FILE* sievew = fopen(sieveoutw, "w");
							int removed = 0;
							while(fgets(line, 100, sieve)!=NULL)
							{	if(strchr(line, '*')!=NULL)
									removed = 0;
								if(strcmp(line, family)==0)
									removed = 1;
								if(removed==0)
									fprintf(sievew, "%s", line);
							}
							fclose(sieve);
							fclose(sievew);
							remove(sieveout);
							rename(sieveoutw, sieveout);

							continue;
						}

						mpz_set_str(p, candidate, base);
						result = mpz_probab_prime_p(p, 1);
						if(result>0)
						{	printf("%s%c^(%d)%s (base %d) probably prime\n", start, middle[0], num, end, base);
							FILE* append = fopen(kernelfilename, "a");
							fprintf(append, "%s\n", candidate);
							fclose(append);

							char sieveoutw[100];
							sieve = fopen(sieveout, "r");
							sprintf(sieveoutw, "data/tmp-sieve.%d.out.txt", base);
							FILE* sievew = fopen(sieveoutw, "w");
							int removed = 0;
							while(fgets(line, 100, sieve)!=NULL)
							{	if(strchr(line, '*')!=NULL)
									removed = 0;
								if(strcmp(line, family)==0)
									removed = 1;
								if(removed==0)
									fprintf(sievew, "%s", line);
							}
							fclose(sieve);
							fclose(sievew);
							remove(sieveout);
							rename(sieveoutw, sieveout);
						}
						else
						{	//printf("%s%c^(%d)%s (base %d) not prime\n", start, middle[0], num, end, base);
							fprintf(out, "%s%c*%s\n", start, middle[0], end);
							count++;

							char sieveoutw[100];
							sieve = fopen(sieveout, "r");
							sprintf(sieveoutw, "data/tmp-sieve.%d.out.txt", base);
							FILE* sievew = fopen(sieveoutw, "w");
							while(fgets(line, 100, sieve)!=NULL)
							{	if(strcmp(line, family)==0)
								{	fprintf(sievew, "%s", line);
									fgets(line, 100, sieve);
								}
								else
									fprintf(sievew, "%s", line);
							}
							fclose(sieve);
							fclose(sievew);
							remove(sieveout);
							rename(sieveoutw, sieveout);
						}
					}
					fclose(out);
					fclose(in);
					remove(infilename);
					rename(outfilename, infilename);
				}
			}
			(void)closedir(dp);
		}
		else
			perror ("Couldn't open the directory");
	}

	mpz_clear(p);

	return 0;
}
