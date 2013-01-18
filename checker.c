#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>
#include "pprime_p.c"
#include <time.h>

int main(int argc, char** argv)
{	
	int result;
	clock_t begin, end;	
	double time_spent;
	double mrtime;
	int i,j,m=1000000, base = atoi(argv[1]); 
	char* pr = (char*)malloc((m>>3)+1); 
	char str[50000];
	if(pr==NULL) 
		exit(1); 
	memset(pr,255,(m>>3)+1); 

	for(i=2; i*i<=m; i++)
		if(pr[i>>3]&(1<<(i&7)))
			for(j=i*i; j<m; j+=i)
				pr[j>>3] &= ~(1<<(j&7));
			/*for(j=m/i;j>=i;j--)
				if(pr[j>>3]&(1<<(j&7)))
					pr[(i*j)>>3]&=~(1<<((i*j)&7));*/

	mpz_t p;
	mpz_init(p);

	begin = clock();

	strcpy(str, argv[2]);
	for(i=0; i<5000; i++)
	{	strcpy(str+i+strlen(argv[2]), argv[4]);

		//if(i>=11000)
		{	mpz_set_str(p, str, base);
			printf("Testing %d...\n", i);
			//begin = clock();
			result = mpz_probab_prime_p_mod(p, 2, &pr, &m, &mrtime);
			//end = clock();
			//time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
			//printf("time: %f sec", time_spent);
			if(result>0)
			{	printf("index %u probably prime\n", i);
				printf("string: %s\n", str);
				printf("width: %d\n", (int)strlen(str));
				break;
			}
			//printf("\n");
		}

		str[i+strlen(argv[2])] = argv[3][0];
	}

	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	mpz_clear(p);
	free(pr);
	printf("total time spent:    %f sec\n", time_spent);
	return 0;
}
