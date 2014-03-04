#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gmp.h>
#define MAXSTRING 1100

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

	if(argc==1)
	{	printf("This program searches unsolved simple families\n");
		printf("for prime candidates starting from exponent n\n");
		printf("where n is given as a command-line argument\n");
		return 0;
	}

	mpz_t p;
	mpz_init(p);

	for(int num=atoi(argv[1]); num<1000; num++)
	{	begin = clock();
		dp = opendir("./data");
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
					int n = atoi(filename+9);
					sprintf(infilename, "data/%s", ep->d_name);
					FILE* in = fopen(infilename, "r");
					sprintf(outfilename, "data/tmp-%s", ep->d_name);
					FILE* out = fopen(outfilename, "w");
					char line[100];
					char start[100];
					char middle;
					char end[100];
					char candidate[MAXSTRING];
					while(fgets(line, 100, in)!=NULL)
					{	int l = (int)(strchr(line, '*')-line);
						middle = line[l-1];
						//printf("%s", line);
						line[strlen(line)-1] = '\0';
						line[l-1] = '\0';
						strcpy(start, line);
						strcpy(end, line+l+1);
						//printf("base: %d start: %s middle: %c end: %s\n", n, start, middle, end);
						strcpy(candidate, start);
						for(int j=0; j<num; j++)
							sprintf(candidate, "%s%c", candidate, middle);
						strcat(candidate, end);
						//printf("candidate: %s\n", candidate);

						char kernelfilename[100];
						sprintf(kernelfilename, "data/minimal.%d.txt", n);

						FILE* kernel = fopen(kernelfilename, "r");
						char prime[MAXSTRING];
						int hassubword = 0;
						//printf("Checking %s%c*%s (base %d)...\n", start, middle, end, n);
						while(fgets(prime, MAXSTRING, kernel)!=NULL)
						{	prime[strlen(prime)-1] = '\0';
							int k;
							if(subword(prime, start, middle, end, &k)==1)
							{	if(k<=num)
								{	hassubword = 1;
									break;
								}
							}
						}
						fclose(kernel);

						if(hassubword)
						{	//printf("%s%c^(%d)%s (base %d) has a kernel subword %s\n", start, middle, num, end, n, prime);
							continue;
						}

						mpz_set_str(p, candidate, n);
						result = mpz_probab_prime_p(p, 25);
						if(result>0)
						{	//gmp_printf("%s%c^(%d)%s (base %d) (%Zd) probably prime\n", start, middle, num, end, n, p);
							printf("%s%c^(%d)%s (base %d) probably prime\n", start, middle, num, end, n);
							FILE* append = fopen(kernelfilename, "a");
							fprintf(append, "%s\n", candidate);
							fclose(append);
						}
						else
						{	fprintf(out, "%s%c*%s\n", start, middle, end);
							count++;
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

		printf("FINISHED LEVEL %d, COUNT REMAINING %d, TIME %f\n", num, count, (double)(clock()-begin)/CLOCKS_PER_SEC);
	}

	mpz_clear(p);

	return 0;
}
