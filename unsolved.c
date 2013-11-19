#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <gmp.h>

// Compute smallest n>=0 such that (a*b^n+c)/d = 0 (mod p)
// returns -1 if no such n exists
int discretelog(mpz_t a, int b, mpz_t c, int d, mpz_t p)
{	int n;
	mpz_t first, cur, temp;
	mpz_inits(first, cur, temp, NULL);

	mpz_add(first, a, c);
	mpz_divexact_ui(first, first, d);
	mpz_mod(first, first, p);
	mpz_set(cur, first);
	
	for(n=1; ; n++)
	{	mpz_mul_ui(cur, cur, b);
		mpz_submul_ui(cur, c, b-1);
		mpz_mod(cur, cur, p);
		if(mpz_sgn(cur)==0)
			break;
		if(mpz_cmp(cur, first)==0)
		{	n = -1;
			break;
		}
	}

	mpz_clears(first, cur, temp, NULL);

	return n;
}

// Compute smallest n>0 such that (a*b^n+c)/d = (a+c)/d (mod p)
int order(mpz_t a, int b, mpz_t c, int d, mpz_t p)
{	int n;
	mpz_t first, cur, temp;
	mpz_inits(first, cur, temp, NULL);

	mpz_add(first, a, c);
	mpz_divexact_ui(first, first, d);
	mpz_mod(first, first, p);
	mpz_set(cur, first);
	
	for(n=1; ; n++)
	{	mpz_mul_ui(cur, cur, b);
		mpz_submul_ui(cur, c, b-1);
		mpz_mod(cur, cur, p);
		if(mpz_cmp(cur, first)==0)
			break;
	}

	mpz_clears(first, cur, temp, NULL);

	return n;
}

int main(int argc, char** argv)
{
	DIR *dp;
	struct dirent *ep;

	mpz_t one;
	mpz_init_set_ui(one, 1);

	mpz_t p;
	mpz_init(p);
	mpz_set_ui(p, 31337);

	dp = opendir("./data");
	int count=0;
	if(dp != NULL)
	{	while(ep = readdir(dp))
		{	char filename[100];
			strcpy(filename, ep->d_name);
			filename[8] = '\0';
			if(strcmp(filename, "unsolved")==0)
			{	strcpy(strchr(filename+9, '.'), "\0");
				int base = atoi(filename+9);
				if(base!=23)
					continue;
				printf("base %d: [%d]\n", base, order(one, base, one, 1, p));
				sprintf(filename, "data/%s", ep->d_name);
				FILE* in = fopen(filename, "r");
				char line[100];
				char start[100];
				char middle[2];
				char end[100];
				//char candidate[MAXSTRING];
				while(fgets(line, 100, in)!=NULL)
				{	count++;
					int l = (int)(strchr(line, '*')-line);
					middle[0] = line[l-1];
					middle[1] = '\0';
					//printf("%s", line);
					line[strlen(line)-1] = '\0';
					line[l-1] = '\0';
					strcpy(start, line);
					strcpy(end, line+l+1);
					//printf("%d - base: %d start: %s middle: %s end: %s\n", count, n, start, middle, end);

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
					int n = discretelog(temp, base, temp3, (base-1)/g, p);
					int k = order(temp, base, temp3, (base-1)/g, p);
					if(mpz_sgn(temp2)>=0)
						if((base-1)/g==1)
							gmp_printf("%s(%s)^n%s = %Zd*%d^n-%Zd [%d, %d]\n", start, middle, end, temp, base, temp2, n, k);
						else
							gmp_printf("%s(%s)^n%s = (%Zd*%d^n-%Zd)/%d [%d, %d]\n", start, middle, end, temp, base, temp2, (base-1)/g, n, k);
					else
						if((base-1)/g==1)
							gmp_printf("%s(%s)^n%s = %Zd*%d^n+%Zd [%d, %d]\n", start, middle, end, temp, base, temp3, n, k);
						else
							gmp_printf("%s(%s)^n%s = (%Zd*%d^n+%Zd)/%d [%d, %d]\n", start, middle, end, temp, base, temp3, (base-1)/g, n, k);
					mpz_clears(x, y, z, temp, temp2, temp3, temp10, NULL);
					//strcpy(candidate, start);
					//for(int j=0; j<10; j++)
					//	sprintf(candidate, "%s%c", candidate, middle);
					//strcat(candidate, end);
					//printf("candidate: %s\n", candidate);
				}
				fclose(in);
			}
		}
		(void)closedir(dp);
	}
	else
		perror("Couldn't open the directory");

	mpz_clear(p);
	mpz_clear(one);

	return 0;
}
