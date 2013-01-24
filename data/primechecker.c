#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <gmp.h>
#include "../pprime_p.c"
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
		else if(l==strlen(end)+1)
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
	/*int k;
	int n = subword("11111111111111111", "19", '1', "", &k);
	printf("%d %d\n", n, k);*/

	DIR *dp;
	struct dirent *ep;     

	int result;
	clock_t begin, end;	
	double time_spent;
	double mrtime;
	long i, j, m=1000000; 
	char* pr = (char*)malloc((m>>3)+1); 
	if(pr==NULL)
		exit(1); 
	memset(pr,255,(m>>3)+1); 

	for(i=2; i*i<=m; i++)
		if(pr[i>>3]&(1<<(i&7)))
			for(j=i*i; j<m; j+=i)
				pr[j>>3] &= ~(1<<(j&7));

	pr[0] &= ~2;

	mpz_t p;
	mpz_init(p);

	char numline[100] = "0";
	FILE* in = fopen("num", "r");
	if(in!=NULL)
	{	fgets(numline, 100, in);
		fclose(in);
	}

	for(int num=atoi(numline); num<49950; num++)
	{	begin = clock();
		dp = opendir("./");
		int count=0;
		if(dp != NULL)
		{	while(ep = readdir(dp))
			{	char filename[100];
				char mrfilename[100];
				char tmpmrfilename[100];
				strcpy(filename, ep->d_name);
				filename[8] = '\0';
				if(strcmp(filename, "unsolved")==0)
				{	strcpy(mrfilename, "mrtime");
					strcat(mrfilename, filename+9);
					strcpy(tmpmrfilename, "tmp-");
					strcat(tmpmrfilename, mrfilename);
					strcpy(strchr(filename+9, '.'), "\0");
					int n = atoi(filename+9);
					//printf("base %d:\n", n);
					FILE* in = fopen(ep->d_name, "r");
					FILE* mrin = fopen(mrfilename, "r");
					strcpy(filename, "tmp-");
					strcat(filename, ep->d_name);
					FILE* out = fopen(filename, "w");
					FILE* mrout = fopen(tmpmrfilename, "w");
					char line[100];
					char mrline[100];
					char start[100];
					char middle;
					char end[100];
					char candidate[MAXSTRING];
					while(fgets(line, 100, in)!=NULL)
					{	if(mrin==NULL)
							mrtime = 0;
						else
						{	fgets(mrline, 100, mrin);
							mrtime = atof(mrline);
						}
						int l = (int)(strchr(line, '*')-line);
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
						sprintf(kernelfilename, "kernel.%d.txt", n);

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
						result = mpz_probab_prime_p_mod(p, 2, &pr, &m, &mrtime);
						if(result>0)
						{	//gmp_printf("%s%c^(%d)%s (base %d) (%Zd) probably prime\n", start, middle, num, end, n, p);
							printf("%s%c^(%d)%s (base %d) probably prime\n", start, middle, num, end, n);
							FILE* append = fopen(kernelfilename, "a");
							fprintf(append, "%s\n", candidate);
							fclose(append);
						}
						else
						{	fprintf(out, "%s%c*%s\n", start, middle, end);
							fprintf(mrout, "%f\n", mrtime);
							count++;
						}
					}
					fclose(out);
					fclose(mrout);
					fclose(in);
					if(mrin!=NULL)
						fclose(mrin);
					remove(ep->d_name);
					rename(filename, ep->d_name);
					rename(tmpmrfilename, mrfilename);
				}
			}
			(void)closedir(dp);
		}
		else
			perror ("Couldn't open the directory");
		FILE* out = fopen("num", "w");
		fprintf(out, "%d", num);
		fclose(out);
		printf("FINISHED LEVEL %d, COUNT REMAINING %d, TIME %f\n", num, count, (double)(clock()-begin)/CLOCKS_PER_SEC);
	}

	mpz_clear(p);
	free(pr);

	return 0;
}
