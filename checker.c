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

	int count1=0, success1=0;
	int count2=0, success2=0;
	int count4=0, success4=0;
	int count8=0, success8=0;
	int count16=0, success16=0;
	int count32=0, success32=0;

	strcpy(str, argv[2]);
	for(i=0; i<10000; i++)
	{	strcpy(str+i+strlen(argv[2]), argv[4]);

		//if(i>=11000)
		{	mpz_set_str(p, str, base);
			printf("Testing %d...\n", i);
			//begin = clock();
			result = mpz_probab_prime_p_mod(p, 2, &pr, &m, &mrtime, &count1, &count2, &count4, &count8, &count16, &count32, &success1, &success2, &success4, &success8, &success16, &success32);
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
	printf("count1:  %d/%d (%f)\n", success1, count1, (double)success1/(double)count1);
	printf("count2:  %d/%d (%f)\n", success2, count2, (double)success2/(double)count2);
	printf("count4:  %d/%d (%f)\n", success4, count4, (double)success4/(double)count4);
	printf("count8:  %d/%d (%f)\n", success8, count8, (double)success8/(double)count8);
	printf("count16: %d/%d (%f)\n", success16, count16, (double)success16/(double)count16);
	printf("count32: %d/%d (%f)\n", success32, count32, (double)success32/(double)count32);
	return 0;
}
