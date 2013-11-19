#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <gmp.h>

// Compute smallest n>=0 such that (a*b^n+c)/d = 0 (mod p)
// returns -1 if no such n exists
int naivediscretelog(mpz_t a, int b, mpz_t c, int d, mpz_t p)
{	int n;
	mpz_t first, cur, pd, base;
	mpz_inits(first, cur, pd, base, NULL);

	mpz_set_ui(cur, b);
	mpz_mod(cur, cur, p);
	if(mpz_sgn(cur)==0)
	{	mpz_clears(first, cur, pd, NULL);
		return -1;
	}

	mpz_set_ui(base, b);
	mpz_mul_ui(pd, p, d);
	mpz_add(first, a, c);
	mpz_mod(first, first, pd);
	
	for(n=0; ; n++)
	{	mpz_powm_ui(cur, base, n, pd);
		mpz_mul(cur, cur, a);
		mpz_add(cur, cur, c);
		mpz_mod(cur, cur, pd);
		if(mpz_sgn(cur)==0)
			break;
		if(mpz_cmp(cur, first)==0 && n>0)
		{	n = -1;
			break;
		}
	}

	mpz_clears(first, cur, pd, base, NULL);

	return n;
}

// Compute smallest n>=0 such that (a*b^n+c)/d = 0 (mod p)
// returns -1 if no such n exists
int discretelog(mpz_t a, int b, mpz_t c, int d, mpz_t p)
{	int n;
	mpz_t first, cur, pd;
	mpz_inits(first, cur, pd, NULL);

	mpz_set_ui(cur, b);
	mpz_mod(cur, cur, p);
	if(mpz_sgn(cur)==0)
	{	mpz_clears(first, cur, pd, NULL);
		return -1;
	}

	mpz_mul_ui(pd, p, d);
	mpz_add(first, a, c);
	mpz_mod(first, first, pd);
	mpz_set(cur, first);
	
	for(n=0; ; n++)
	{	if(mpz_sgn(cur)==0)
			break;
		if(mpz_cmp(cur, first)==0 && n>1)
		{	n = -1;
			break;
		}
		mpz_mul_ui(cur, cur, b);
		mpz_submul_ui(cur, c, b-1);
		mpz_mod(cur, cur, pd);
	}

	mpz_clears(first, cur, pd, NULL);

	return n;
}

// Compute smallest n>0 such that b^n = 1 (mod p)
// returns -1 if no such n exists
int order(int b, mpz_t p)
{	int n;
	mpz_t one, cur;
	mpz_inits(one, cur, NULL);

	mpz_set_ui(one, 1);
	mpz_set_ui(cur, 1);
	
	for(n=1; ; n++)
	{	mpz_mul_ui(cur, cur, b);
		mpz_mod(cur, cur, p);
		if(mpz_cmp(cur, one)==0)
			break;
		if(mpz_sgn(cur)==0)
		{	n = -1;
			break;
		}
	}

	mpz_clears(one, cur, NULL);

	return n;
}

// Compute smallest n>0 such that (a*b^n+c)/d = (a+c)/d (mod p)
// returns -1 if no such n exists
int naivegenorder(mpz_t a, int b, mpz_t c, int d, mpz_t p)
{	int n;
	mpz_t first, cur, pd, base;
	mpz_inits(first, cur, pd, base, NULL);

	mpz_set_ui(cur, b);
	mpz_mod(cur, cur, p);
	if(mpz_sgn(cur)==0)
	{	mpz_clears(first, cur, pd, base, NULL);
		return -1;
	}

	mpz_set_ui(base, b);
	mpz_mul_ui(pd, p, d);
	mpz_add(first, a, c);
	mpz_mod(first, first, pd);

	for(n=1; ; n++)
	{	mpz_powm_ui(cur, base, n, pd);
		mpz_mul(cur, cur, a);
		mpz_add(cur, cur, c);
		mpz_mod(cur, cur, pd);
		if(mpz_cmp(cur, first)==0)
			break;
	}

	mpz_clears(first, cur, pd, base, NULL);

	return n;
}

// Compute smallest n>0 such that (a*b^n+c)/d = (a+c)/d (mod p)
// returns -1 if no such n exists
int genorder(mpz_t a, int b, mpz_t c, int d, mpz_t p)
{	int n;
	mpz_t first, cur, pd;
	mpz_inits(first, cur, pd, NULL);

	mpz_set_ui(cur, b);
	mpz_mod(cur, cur, p);
	if(mpz_sgn(cur)==0)
	{	mpz_clears(first, cur, pd, NULL);
		return -1;
	}

	mpz_mul_ui(pd, p, d);
	mpz_add(first, a, c);
	mpz_mod(first, first, pd);
	mpz_set(cur, first);

	for(n=1; ; n++)
	{	mpz_mul_ui(cur, cur, b);
		mpz_submul_ui(cur, c, b-1);
		mpz_mod(cur, cur, pd);
		if(mpz_cmp(cur, first)==0)
			break;
	}

	mpz_clears(first, cur, pd, NULL);

	return n;
}

int main(int argc, char** argv)
{
	DIR *dp;
	struct dirent *ep;

	mkdir("work", S_IRWXU);

	mpz_t p;
	mpz_inits(p, NULL);

	dp = opendir("./data");
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
				//printf("base %d: [%d]\n", base, order(base, p));
				sprintf(filename, "data/%s", ep->d_name);
				FILE* in = fopen(filename, "r");
				char line[100];
				char start[100];
				char middle[2];
				char end[100];
				int count = 0;
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
					// Print family
					/*if(mpz_sgn(temp2)>=0)
						if((base-1)/g==1)
							gmp_printf("%s(%s)^n%s = %Zd*%d^n-%Zd\n", start, middle, end, temp, base, temp2);
						else
							gmp_printf("%s(%s)^n%s = (%Zd*%d^n-%Zd)/%d\n", start, middle, end, temp, base, temp2, (base-1)/g);
					else
						if((base-1)/g==1)
							gmp_printf("%s(%s)^n%s = %Zd*%d^n+%Zd\n", start, middle, end, temp, base, temp3);
						else
							gmp_printf("%s(%s)^n%s = (%Zd*%d^n+%Zd)/%d\n", start, middle, end, temp, base, temp3, (base-1)/g);*/

					char exponents[60000];
					for(int i=0; i<60000; i++)
						exponents[i] = 1;
					for(mpz_set_ui(p, 2); mpz_cmp_ui(p, 101)<=0; mpz_nextprime(p, p))
					{	int n = discretelog(temp, base, temp3, (base-1)/g, p);
						int n2 = naivediscretelog(temp, base, temp3, (base-1)/g, p);
						//int ord = order(base, p);
						int ord = genorder(temp, base, temp3, (base-1)/g, p);
						int ord2 = naivegenorder(temp, base, temp3, (base-1)/g, p);
						if(n!=n2 || ord!=ord2)
							gmp_printf("a: %Zd b: %d c: %Zd d: %d p: %Zd | %d %d | %d %d\n", temp, base, temp3, (base-1)/g, p, n, n2, ord, ord2);
						if(n!=-1 && ord!=-1)
							for(int j=n; j<60000; j+=ord)
								exponents[j] = 0;
						//else
						//	printf("%d %d\n", n, ord);
					}

					/*sprintf(filename, "work/%d-%d.txt", base, count);
					FILE* out = fopen(filename, "w");
					gmp_fprintf(out, "%Zd %Zd %d\n", temp, temp3, (base-1)/g);
					for(int i=0; i<60000; i++)
						if(exponents[i]==1)
							fprintf(out, "%d\n", i);
					fclose(out);*/

					mpz_clears(x, y, z, temp, temp2, temp3, temp10, NULL);
				}
				fclose(in);
			}
		}
		(void)closedir(dp);
	}
	else
		perror("Couldn't open the directory");

	mpz_clears(p);

	return 0;
}
