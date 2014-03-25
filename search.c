#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gmp.h>
#define MAXSTRING 1200000

FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);

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

	mpz_t p;
	mpz_init(p);

	if(argc<=2)
	{	printf("After sieving has been done, this program uses LLR\n");
		printf("to search for prime candidates between exponents\n");
		printf("n and m, given on the command-line\n");
		printf("\nNOTE: The program llr must be located in the base directory\n");
		return 0;
	}

	for(int i=atoi(argv[1]); i<=atoi(argv[2]); i++)
	{	dp = opendir("./data");
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

						// Find an exponent to test
						int num = -1;
						char sievefilename[100], sievetmpfilename[100];
						sprintf(sievefilename, "data/sieve.%d.txt", base);
						sprintf(sievetmpfilename, "data/tmp-sieve.%d.txt", base);
						FILE* sieve = fopen(sievefilename, "r");
						if(sieve!=NULL)
						{	while(fgets(line, 100, sieve)!=NULL)
							{	if(strcmp(line, family)==0)
								{	while(num<i && fgets(line, 100, sieve)!=NULL && strchr(line, '*')==NULL)
										num = atoi(line);
									break;
								}
							}
							fclose(sieve);
						}

						if(num!=i)
						{	fprintf(out, "%s%c*%s\n", start, middle[0], end);
							continue;
						}

						strcpy(candidate, start);
						for(int j=0; j<num; j++)
							sprintf(candidate, "%s%c", candidate, middle[0]);
						strcat(candidate, end);

						char kernelfilename[100];
						sprintf(kernelfilename, "data/minimal.%d.txt", base);

						FILE* kernel = fopen(kernelfilename, "r");
						char prime[MAXSTRING];
						int hassubword = 0;
						printf("Checking %s%c^(%d)%s (base %d)...\n", start, middle[0], num, end, base);
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

							// Remove the family from the sieve file
							sieve = fopen(sievefilename, "r");
							FILE* sieveout = fopen(sievetmpfilename, "w");
							char sieveline[100];
							int thisfamily = 0;
							while(fgets(sieveline, 100, sieve)!=NULL)
							{	if(strchr(sieveline, '*')!=NULL)
									thisfamily = 0;
								if(strcmp(sieveline, family)==0)
									thisfamily = 1;
								if(thisfamily==0)
									fprintf(sieveout, "%s", sieveline);
							}
							fclose(sieve);
							fclose(sieveout);
							remove(sievefilename);
							rename(sievetmpfilename, sievefilename);

							continue;
						}

						FILE* llrfile = fopen("llr.in", "w");
						fprintf(llrfile, "ABC ($a*$b^$c$d)/$e\n");
						gmp_fprintf(llrfile, "%Zd %d %d %+Zd %d\n", temp, base, num, temp3, (base-1)/g);
						fclose(llrfile);

						char output[1000000];
						FILE* llrprocess = popen("./llr llr.in -d -oOutputIterations=1000000", "r");
						int n = fread(output, 1, 999999, llrprocess);
						output[n] = '\0';
						pclose(llrprocess);
						printf("%s", strstr(output, "\r(")!=NULL ? strstr(output, "\r(") : output);

						if(strstr(output, "PRP")!=NULL)
						{	// Add prime to set of minimal primes
							FILE* append = fopen(kernelfilename, "a");
							fprintf(append, "%s\n", candidate);
							fclose(append);

							// Remove the family from the sieve file
							sieve = fopen(sievefilename, "r");
							FILE* sieveout = fopen(sievetmpfilename, "w");
							char sieveline[100];
							int thisfamily = 0;
							while(fgets(sieveline, 100, sieve)!=NULL)
							{	if(strchr(sieveline, '*')!=NULL)
									thisfamily = 0;
								if(strcmp(sieveline, family)==0)
									thisfamily = 1;
								if(thisfamily==0)
									fprintf(sieveout, "%s", sieveline);
							}
							fclose(sieve);
							fclose(sieveout);
							remove(sievefilename);
							rename(sievetmpfilename, sievefilename);
						}
						else if(strstr(output, "not prime")!=NULL)
						{	// Family is still unsolved
							fprintf(out, "%s%c*%s\n", start, middle[0], end);

							// Remove the exponent just tested from the sieve file
							sieve = fopen(sievefilename, "r");
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
						else
						{	fprintf(out, "%s%c*%s\n", start, middle[0], end);
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
			perror("Couldn't open the directory");
	}

	mpz_clear(p);

	return 0;
}
