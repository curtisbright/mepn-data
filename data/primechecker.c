#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <gmp.h>
#include "../pprime_p.c"

int main(int argc, char** argv)
{
	DIR *dp;
	struct dirent *ep;     

	int result;
	clock_t begin, end;	
	double time_spent;
	double mrtime;
	int i, j, m=1000000; 
	char* pr = (char*)malloc((m>>3)+1); 
	char str[50000];
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

	for(int num=0; num<10; num++)
	{	dp = opendir("./");
		if(dp != NULL)
		{	while(ep = readdir(dp))
			{	char str[20];
				strcpy(str, ep->d_name);
				str[8] = '\0';
				if(strcmp(str, "unsolved")==0)
				{	strcpy(strchr(str+9, '.'), "\0");
					int n = atoi(str+9);
					//printf("base %d:\n", n);
					FILE* in = fopen(ep->d_name, "r");
					char line[100];
					char start[100];
					char middle;
					char end[100];
					char candidate[10000];
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

						//begin = clock();

						mpz_set_str(p, candidate, n);
						result = mpz_probab_prime_p_mod(p, 2, &pr, &m, &mrtime);
						if(result>0)
						{	//printf("index %u probably prime\n", num);
							//printf("base: %d start: %s middle: %c end: %s\n", n, start, middle, end);
							//printf("string: %s\n", candidate);
							//printf("width: %d\n", (int)strlen(candidate));
							printf("%s (base %d) probably prime\n", candidate, n);
							break;
						}
						//end = clock();
						//time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
					}
					fclose(in);
				}
			}
			(void)closedir(dp);
		}
		else
			perror ("Couldn't open the directory");
	}

	mpz_clear(p);
	free(pr);

	return 0;
}
