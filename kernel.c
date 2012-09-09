#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>
#define MAXSTRING 20000

typedef struct
{	int len;
	int* numrepeats;
	char* digit;
	char** repeats;
} family;

typedef struct
{	int size;
	char** primes;
} kernel;

typedef struct
{	int size;
	family* fam;
} list;

void familystring(char* str, family p);
void clearfamily(family* f);
void copyfamily(family* newf, family f);
void adddigit(family* f, char d, char* r, int n);
void familyinit(family* p);

int base;
int depth;
kernel K;
int prsize;
char* pr;
list unsolved;

void listinit(list* l)
{	l->size = 0;
	l->fam = NULL;
}

void copylist(list* out, list in)
{	out->size = in.size;
	out->fam = malloc(in.size*sizeof(family));
	for(int i=0; i<in.size; i++)
		copyfamily(&(out->fam[i]), in.fam[i]);
		//out->fam[i] = in.fam[i];
}

void clearlist(list* l)
{	for(int i=0; i<l->size; i++)
		clearfamily(&(l->fam[i]));
	free(l->fam);
	listinit(l);
}

int samefamily(family f, family g)
{	if(f.len!=g.len)
		return 0;
	for(int i=0; i<f.len; i++)
	{	if(f.digit[i]!=g.digit[i])
			return 0;
		if(f.numrepeats[i]!=g.numrepeats[i])
			return 0;
		for(int j=0; j<f.numrepeats[i]; j++)
			if(f.repeats[i][j]!=g.repeats[i][j])
				return 0;
	}
	return 1;
}

void addtolist(list* l, family f)
{	if((l->size)>0 && samefamily(f, l->fam[(l->size)-1]))
		return;
	int size = ++l->size;
	l->fam = (family*)realloc(l->fam, size*sizeof(family));
	familyinit(&((l->fam)[size-1]));
	copyfamily(&((l->fam)[size-1]), f);
	//l->fam[size-1] = f;
	/*char str[MAXSTRING];
	familystring(str, f);
	printf("added to list: %s\n", str);*/
}

void kernelinit()
{	K.size = 0;
	K.primes = NULL;
}

void copykernel(kernel* temp)
{	temp->size = K.size;
	temp->primes = malloc(K.size*sizeof(char*));
	for(int i=0; i<K.size; i++)
	{	temp->primes[i] = malloc(MAXSTRING);
		strcpy(temp->primes[i], K.primes[i]);
	}
}

void addtokernel(char* p)
{	int size = ++K.size;
	K.primes = realloc(K.primes, size*sizeof(char*));
	K.primes[size-1] = p;
	//printf("added to kernel: %s\n", p);
}

void clearkernel()
{	for(int i=0; i<K.size; i++)
		free(K.primes[i]);
	free(K.primes);
	kernelinit();
}

int nosubword(char* p)
{	for(int i=0; i<K.size; i++)
	{	int k = 0;
		for(int j=0; j<strlen(p); j++)
		{	if(K.primes[i][k]==p[j])
				k++;
			if(k==strlen(K.primes[i]))
				return 0;
		}
	}
	return 1;
}

int nosubwordskip(char* p, int skip)
{	for(int i=0; i<K.size; i++)
	{	if(i==skip)
			continue;
		int k = 0;
		for(int j=0; j<strlen(p); j++)
		{	if(K.primes[i][k]==p[j])
				k++;
			if(k==strlen(K.primes[i]))
				return 0;
		}
	}
	return 1;
}

int isprime(char* p)
{	mpz_t temp;
	mpz_init(temp);
	mpz_set_str(temp, p, base);
	if(mpz_probab_prime_p(temp, 1, pr, prsize) > 0)
	{	//gmp_printf("%Zd is prime\n", temp);
		mpz_clear(temp);
		return 1;
	}
	else
	{	//gmp_printf("%Zd is not prime\n", temp);
		mpz_clear(temp);
		return 0;
	}
}

int newminimal(char* p)
{	if(!nosubword(p))
		return 0;
	return isprime(p);
}

void familyinit(family* p)
{	p->len = 0;
	p->numrepeats = NULL;
	p->digit = NULL;
	p->repeats = NULL;
}

void adddigit(family* f, char d, char* r, int n)
{	int len = ++f->len;
	f->digit = realloc(f->digit, len*sizeof(char));
	f->digit[len-1] = d;
	f->numrepeats = realloc(f->numrepeats, len*sizeof(int));
	f->numrepeats[len-1] = n;
	f->repeats = realloc(f->repeats, len*sizeof(char*));
	f->repeats[len-1] = r;
}

void clearfamily(family* f)
{	free(f->digit);
	for(int i=0; i<f->len; i++)
		free(f->repeats[i]);
	free(f->numrepeats);
	free(f->repeats);
	familyinit(f);
}

char digitchar(unsigned char digit)
{	if(digit==255)
		return 0;
	else if(digit>=10)
		return digit+'A'-10;
	else
		return digit+'0';
}

void familystring(char* str, family p)
{	sprintf(str, "%c", 0);
	for(int i=0; i<p.len; i++)
	{	sprintf(str, "%s%c", str, digitchar(p.digit[i]));
		if(p.numrepeats[i]>0)
		{	sprintf(str, "%s{", str);
			for(int j=0; j<p.numrepeats[i]; j++)
				sprintf(str, "%s%c", str, digitchar(p.repeats[i][j]));
			sprintf(str, "%s}*", str);
		}
	}
}

void startinstancestring(char* str, family p, int length)
{	sprintf(str, "%c", 0);
	for(int i=0; i<=length; i++)
		sprintf(str, "%s%c", str, digitchar(p.digit[i]));
}

void endinstancestring(char* str, family p, int start)
{	sprintf(str, "%c", 0);
	for(int i=start+1; i<p.len; i++)
		sprintf(str, "%s%c", str, digitchar(p.digit[i]));
}

void emptyinstancestring(char* str, family p)
{	sprintf(str, "%c", 0);
	for(int i=0; i<p.len; i++)
		sprintf(str, "%s%c", str, digitchar(p.digit[i]));
}

void instancestring(char* str, family p, int x, int y)
{	sprintf(str, "%c", 0);
	for(int i=0; i<p.len; i++)
	{	sprintf(str, "%s%c", str, digitchar(p.digit[i]));
		if(i==x)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x][y]));
	}
}

void doubleinstancestring(char* str, family p, int x1, int y1, int x2, int y2)
{	sprintf(str, "%c", 0);
	for(int i=0; i<p.len; i++)
	{	sprintf(str, "%s%c", str, digitchar(p.digit[i]));
		if(i==x1)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x1][y1]));
		if(i==x2)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x2][y2]));
	}
}

void tripleinstancestring(char* str, family p, int x1, int y1, int x2, int y2, int x3, int y3)
{	sprintf(str, "%c", 0);
	for(int i=0; i<p.len; i++)
	{	sprintf(str, "%s%c", str, digitchar(p.digit[i]));
		if(i==x1)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x1][y1]));
		if(i==x2)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x2][y2]));
		if(i==x3)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x3][y3]));
	}
}

void copyfamily(family* newf, family f)
{	for(int i=0; i<f.len; i++)
	{	char* repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
		memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
		adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
	}
}

int hasdivisor(family p)
{	mpz_t gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7;
	mpz_inits(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
	char str[MAXSTRING];
	int numrepeats = 0;
	emptyinstancestring(str, p);
	mpz_set_str(gcd, str, base);
	for(int i=0; i<p.len; i++)
	{	for(int j=0; j<p.numrepeats[i]; j++)
		{	instancestring(str, p, i, j);
			mpz_set_str(temp, str, base);
			mpz_gcd(gcd, gcd, temp);
		}
		if(p.numrepeats[i]>0)
			numrepeats++;
	}

	if(numrepeats==0)
	{	
#ifdef PRINTDIVISOR
		familystring(str, p);
		printf("%s is trivial\n", str);
#endif
		mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
		return 0;
	}

	if(mpz_cmp_ui(gcd, 1)>0)
	{	
#ifdef PRINTDIVISOR
		familystring(str, p);
		gmp_printf("%s has a divisor %Zd\n", str, gcd);
#endif
		mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
		return 1;
	}

	if(numrepeats<3)
	{	emptyinstancestring(str, p);
		mpz_set_str(gcd1, str, base);
		for(int i=0; i<p.len; i++)
			for(int j=0; j<p.numrepeats[i]; j++)
				for(int k=0; k<p.len; k++)
					for(int l=0; l<p.numrepeats[k]; l++)
					{	doubleinstancestring(str, p, i, j, k, l);
						mpz_set_str(temp, str, base);
						mpz_gcd(gcd1, gcd1, temp);
					}

		int gcdbeenset = 0;
		for(int i=0; i<p.len; i++)
			for(int j=0; j<p.numrepeats[i]; j++)
			{	instancestring(str, p, i, j);
				mpz_set_str(temp, str, base);
				if(gcdbeenset)
					mpz_gcd(gcd2, gcd2, temp);
				else
				{	gcdbeenset = 1;
					mpz_set(gcd2, temp);
				}
			}
		for(int i=0; i<p.len; i++)
			for(int j=0; j<p.numrepeats[i]; j++)
				for(int k=0; k<p.len; k++)
					for(int l=0; l<p.numrepeats[k]; l++)
						for(int m=0; m<p.len; m++)
							for(int n=0; n<p.numrepeats[m]; n++)
							{	tripleinstancestring(str, p, i, j, k, l, m, n);
								mpz_set_str(temp, str, base);
								mpz_gcd(gcd2, gcd2, temp);
							}

		if(mpz_cmp_ui(gcd1, 1)>0 && mpz_cmp_ui(gcd2, 1)>0)
		{	
#ifdef PRINTDIVISOR
			familystring(str, p);
			gmp_printf("%s has two divisors %Zd and %Zd\n", str, gcd1, gcd2);
#endif
			mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
			return 1;
		}
	}

	char end[MAXSTRING], start[MAXSTRING], middle[2];

	if(numrepeats==1)
	{	for(int i=0; i<p.len; i++)
			if(p.numrepeats[i]==1)
			{	endinstancestring(str, p, i);
				int zlen = strlen(str);
				mpz_set_str(z, str, base);
				mpz_set_ui(y, p.repeats[i][0]);
				startinstancestring(str, p, i);
				mpz_set_str(x, str, base);

				//char end[MAXSTRING], start[MAXSTRING], middle[2];
				endinstancestring(end, p, i);
				sprintf(middle, "%c", digitchar(p.repeats[i][0]));
				startinstancestring(start, p, i);
				sprintf(str, "%s%s\n", start, end);
				mpz_set_str(temp, str, base);
				sprintf(str, "%s%s%s\n", start, middle, end);
				mpz_set_str(temp2, str, base);
				sprintf(str, "%s%s%s%s\n", start, middle, middle, end);
				mpz_set_str(temp3, str, base);
				sprintf(str, "%s%s%s%s%s\n", start, middle, middle, middle, end);
				mpz_set_str(temp4, str, base);
				sprintf(str, "%s%s%s%s%s%s\n", start, middle, middle, middle, middle, end);
				mpz_set_str(temp5, str, base);
				sprintf(str, "%s%s%s%s%s%s%s\n", start, middle, middle, middle, middle, middle, end);
				mpz_set_str(temp6, str, base);

				mpz_gcd(temp, temp, temp4);
				mpz_gcd(temp2, temp2, temp5);
				mpz_gcd(temp3, temp3, temp6);

				if(mpz_cmp_ui(temp, 1)>0 && mpz_cmp_ui(temp2, 1)>0 && mpz_cmp_ui(temp3, 1)>0)
				{	mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
					return 1;
				}

				if(mpz_fdiv_ui(y, base-1)==0)
				{	mpz_divexact_ui(temp, y, base-1);
					mpz_set(temp2, temp);
					mpz_add(temp, temp, x);
					mpz_ui_pow_ui(temp3, base, zlen);
					mpz_mul(temp, temp, temp3);
					mpz_mul(temp2, temp2, temp3);
					mpz_sub(temp2, temp2, z);

					if(mpz_root(temp3, temp, 2)!=0 && mpz_sgn(temp2)>=0 && mpz_root(temp4, temp2, 2)!=0)
					{	mpz_add(temp5, temp3, temp4);
						mpz_sub(temp6, temp3, temp4);
						mpz_set_ui(temp, base);
						if(mpz_cmp_ui(temp5, 1)>0 && mpz_cmp_ui(temp6, 1)>0 && mpz_root(temp, temp, 2)!=0)
						{	
#ifdef PRINTDIVISOR
							familystring(str, p);
							gmp_printf("%s factors as a difference of squares\n", str);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
							return 1;
						}
					}

					if(mpz_root(temp3, temp, 3)!=0 && mpz_root(temp4, temp2, 3)!=0)
					{	mpz_mul(temp5, temp3, temp3);
						mpz_mul(temp6, temp4, temp4);
						mpz_add(temp6, temp6, temp5);
						mpz_mul(temp5, temp3, temp4);
						mpz_add(temp6, temp6, temp5);
						mpz_sub(temp5, temp3, temp4);
						mpz_set_ui(temp, base);
						if(mpz_cmp_ui(temp5, 1)>0 && mpz_cmp_ui(temp6, 1)>0 && mpz_root(temp, temp, 3)!=0)
						{	
#ifdef PRINTDIVISOR
							familystring(str, p);
							gmp_printf("%s factors as a difference of cubes\n", str);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
							return 1;
						}
					}
				}
				else
				{	mpz_set(temp, y);
					mpz_addmul_ui(temp, x, base-1);
					mpz_ui_pow_ui(temp3, base, zlen);
					mpz_mul(temp, temp, temp3);
					mpz_mul(temp2, y, temp3);
					mpz_submul_ui(temp2, z, base-1);

					if(mpz_root(temp3, temp, 2)!=0 && mpz_sgn(temp2)>=0 && mpz_root(temp4, temp2, 2)!=0)
					{	mpz_add(temp5, temp3, temp4);
						mpz_sub(temp6, temp3, temp4);
						mpz_set_ui(temp7, base);
						if(mpz_cmp_ui(temp5, base-1)>0 && mpz_cmp_ui(temp6, base-1)>0 && mpz_root(temp7, temp7, 2)!=0)
						{
#ifdef PRINTDIVISOR
							familystring(str, p);
							gmp_printf("%s factors as a difference of squares\n", str);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
							return 1;
						}
						else if(mpz_cmp_ui(temp5, base-1)>0 && mpz_cmp_ui(temp6, base-1)>0 && mpz_cmp_ui(gcd2, 1)>0)
						{
#ifdef PRINTDIVISORSPECIAL
							familystring(str, p);
							gmp_printf("%s factors as a difference of squares for even n, and has a factor %Zd for odd n\n", str, gcd2);
							gmp_printf("%s(%s)^n%s = %Zd + %d^%d*%Zd*(%d^n-1)/%d + %d^(n+%d)*%Zd = (%Zd*%d^n-%Zd)/%d = (%Zd*%d^(n/2)-%Zd)*(%Zd*%d^(n/2)+%Zd)/%d\n", start, middle, end, z, base, zlen, y, base, base-1, base, zlen, x, temp, base, temp2, base-1, temp3, base, temp4, temp3, base, temp4, base-1);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
							return 1;
						}
					}
					/*if(mpz_fdiv_ui(temp, base)==0)
					{	mpz_divexact_ui(temp, temp, base);
						if(mpz_root(temp3, temp, 2)!=0 && mpz_sgn(temp2)>=0 && mpz_root(temp4, temp2, 2)!=0)
						{	mpz_add(temp5, temp3, temp4);
							mpz_sub(temp6, temp3, temp4);
							mpz_set_ui(temp, base);

							if(mpz_cmp_ui(temp5, base-1)>0 && mpz_cmp_ui(temp6, base-1)>0 && mpz_cmp_ui(gcd1, 1)>0)
							{
#ifdef PRINTDIVISORSPECIAL
								familystring(str, p);
								gmp_printf("%s factors as a difference of squares for odd n, and has a factor %Zd for even n\n", str, gcd1);
#endif
								mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
								return 1;
							}
						}
					}*/

					if(mpz_root(temp3, temp, 3)!=0 && mpz_root(temp4, temp2, 3)!=0)
					{	mpz_mul(temp5, temp3, temp3);
						mpz_mul(temp6, temp4, temp4);
						mpz_add(temp6, temp6, temp5);
						mpz_mul(temp5, temp3, temp4);
						mpz_add(temp6, temp6, temp5);
						mpz_sub(temp5, temp3, temp4);
						mpz_set_ui(temp, base);
						if(mpz_cmp_ui(temp5, 1)>0 && mpz_cmp_ui(temp6, 1)>0 && mpz_root(temp, temp, 3)!=0)
						{	
#ifdef PRINTDIVISOR
							familystring(str, p);
							gmp_printf("%s factors as a difference of cubes\n", str);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
							return 1;
						}
					}
				}
			}
	}

	mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
	return 0;
}

void instancefamily(family* newf, family f, int side, int back)
{	if(back==0)
	{	int firstrepeat = 1;
		for(int i=0; i<f.len; i++)
		{	char* repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
			memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
			if(f.numrepeats[i]>0 && firstrepeat)
			{	if(side==1)
				{	adddigit(newf, f.digit[i], NULL, 0);
					adddigit(newf, 0, repeatscopy, f.numrepeats[i]);
				}
				else if(side==0)
				{	adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
					adddigit(newf, 0, NULL, 0);
				}
				/*else
				{	adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
					repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
					memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
					adddigit(newf, 0, repeatscopy, f.numrepeats[i]);
				}*/
				firstrepeat = 0;
			}
			else
				adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
		}
	}
	else
	{	int lastrepeatpos;
		for(int i=0; i<f.len; i++)
		{	if(f.numrepeats[i]>0)
				lastrepeatpos = i;
		}
		for(int i=0; i<f.len; i++)
		{	char* repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
			memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
			if(i==lastrepeatpos)
			{	if(side==1)
				{	adddigit(newf, f.digit[i], NULL, 0);
					adddigit(newf, 0, repeatscopy, f.numrepeats[i]);
				}
				else if(side==0)
				{	adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
					adddigit(newf, 0, NULL, 0);
				}
				/*else
				{	adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
					repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
					memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
					adddigit(newf, 0, repeatscopy, f.numrepeats[i]);
				}*/
			}
			else
				adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
		}
	}
}

int examine(family* f)
{	char* str = malloc(MAXSTRING);
	emptyinstancestring(str, *f);
	if(!nosubword(str))
	{	free(str);
		return 0;
	}
	else if(isprime(str))
	{	addtokernel(str);
		return 0;
	}
	free(str);

	int trivial = 1;
	for(int i=0; i<f->len; i++)
	{	int newnumrepeat = 0;
		for(int j=0; j<f->numrepeats[i]; j++)
		{	char tempstr[MAXSTRING];
			instancestring(tempstr, *f, i, j);
			if(nosubword(tempstr))
				f->repeats[i][newnumrepeat++] = f->repeats[i][j];
		}
		f->numrepeats[i] = newnumrepeat;
		if(newnumrepeat>0)
			trivial = 0;
	}

	if(trivial)
		return 0;

	if(hasdivisor(*f))
		return 0;

	return 1;
}

int split(family* f)
{	for(int i=0; i<f->len; i++)
	{	for(int j=0; j<f->numrepeats[i]; j++)
		{	if(f->numrepeats[i]==1)
				continue;
			char str[MAXSTRING];
			doubleinstancestring(str, *f, i, j, i, j);
			//if(nosubword(str) && isprime(str))
			//	addtokernel(str);
			
			if(!nosubword(str))
			{	
				family copyf;
				familyinit(&copyf);
				copyfamily(&copyf, *f);

				int newnumrepeats = 0;
				int removeddigit = copyf.repeats[i][j];
				for(int k=0; k<copyf.numrepeats[i]; k++)
				{	if(k!=j)
						copyf.repeats[i][newnumrepeats++] = copyf.repeats[i][k];
				}
				copyf.numrepeats[i] = newnumrepeats;

				family newf;
				familyinit(&newf);
				for(int k=0; k<copyf.len; k++)
				{	char* newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
					memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
					adddigit(&newf, copyf.digit[k], newrepeats, copyf.numrepeats[k]);
					if(k==i)
					{	newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf, removeddigit, newrepeats, copyf.numrepeats[k]);
					}
				}

				//clearfamily(f);

				addtolist(&unsolved, copyf);
				addtolist(&unsolved, newf);

				/*char str[MAXSTRING];
				familystring(str, *f);
				printf("%s splits into ", str);
				familystring(str, copyf);
				printf("%s and ", str);
				familystring(str, newf);
				printf("%s\n", str);*/

				clearfamily(&copyf);
				clearfamily(&newf);
			
				return 1;
			}
		}
	}
	addtolist(&unsolved, *f);
	return 0;
}

int split2(family* f)
{	for(int i=0; i<f->len; i++)
	{	for(int j=0; j<f->numrepeats[i]; j++)
		{	for(int k=j+1; k<f->numrepeats[i]; k++)
			{	char str1[MAXSTRING];
				char str2[MAXSTRING];
				doubleinstancestring(str1, *f, i, j, i, k);
				doubleinstancestring(str2, *f, i, k, i, j);
				if(!nosubword(str1) && !nosubword(str2))
				{	family copyf;
					familyinit(&copyf);
					copyfamily(&copyf, *f);
					int newnumrepeats = 0;
					for(int l=0; l<copyf.numrepeats[i]; l++)
					{	if(l!=j)
							copyf.repeats[i][newnumrepeats++] = copyf.repeats[i][l];
					}
					copyf.numrepeats[i] = newnumrepeats;
					addtolist(&unsolved, copyf);

					/*char str[MAXSTRING];
					familystring(str, *f);
					printf("%s splits into ", str);
					familystring(str, copyf);
					printf("%s and ", str);*/

					clearfamily(&copyf);

					familyinit(&copyf);
					copyfamily(&copyf, *f);
					newnumrepeats = 0;
					for(int l=0; l<copyf.numrepeats[i]; l++)
					{	if(l!=k)
							copyf.repeats[i][newnumrepeats++] = copyf.repeats[i][l];
					}
					copyf.numrepeats[i] = newnumrepeats;
					addtolist(&unsolved, copyf);

					/*familystring(str, copyf);
					printf("%s\n", str);*/

					clearfamily(&copyf);

					//clearfamily(f);

					return 1;
				}
				else if((!nosubword(str1)) && (f->numrepeats[i])==2)
				{	family newf;
					familyinit(&newf);
					for(int l=0; l<f->len; l++)
					{	char* newrepeats = malloc(f->numrepeats[l]*sizeof(char));
						memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
						adddigit(&newf, f->digit[l], newrepeats, f->numrepeats[l]);
						if(i==l)
						{	int newnumrepeats = 0;
							int removeddigit = f->repeats[i][j];
							for(int m=0; m<f->numrepeats[i]; m++)
							{	if(m!=j)
									newf.repeats[i][newnumrepeats++] = f->repeats[i][m];
							}
							newf.numrepeats[i] = newnumrepeats;

							newrepeats = malloc(f->numrepeats[l]*sizeof(char));
							memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
							adddigit(&newf, 255, newrepeats, f->numrepeats[l]);

							newnumrepeats = 0;
							removeddigit = f->repeats[i][k];
							for(int m=0; m<f->numrepeats[i]; m++)
							{	if(m!=k)
									newf.repeats[i+1][newnumrepeats++] = f->repeats[i][m];
							}
							newf.numrepeats[i+1] = newnumrepeats;
						}
					}
					addtolist(&unsolved, newf);

					/*char str[MAXSTRING];
					familystring(str, *f);
					printf("%s splits into ", str);
					familystring(str, newf);
					printf("%s\n", str);*/

					clearfamily(&newf);

					return 1;
				}
				else if((!nosubword(str2)) && (f->numrepeats[i])==2)
				{	family newf;
					familyinit(&newf);
					for(int l=0; l<f->len; l++)
					{	char* newrepeats = malloc(f->numrepeats[l]*sizeof(char));
						memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
						adddigit(&newf, f->digit[l], newrepeats, f->numrepeats[l]);
						if(i==l)
						{	int newnumrepeats = 0;
							int removeddigit = f->repeats[i][k];
							for(int m=0; m<f->numrepeats[i]; m++)
							{	if(m!=k)
									newf.repeats[i][newnumrepeats++] = f->repeats[i][m];
							}
							newf.numrepeats[i] = newnumrepeats;

							newrepeats = malloc(f->numrepeats[l]*sizeof(char));
							memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
							adddigit(&newf, 255, newrepeats, f->numrepeats[l]);

							newnumrepeats = 0;
							removeddigit = f->repeats[i][j];
							for(int m=0; m<f->numrepeats[i]; m++)
							{	if(m!=j)
									newf.repeats[i+1][newnumrepeats++] = f->repeats[i][m];
							}
							newf.numrepeats[i+1] = newnumrepeats;
						}
					}
					addtolist(&unsolved, newf);

					/*char str[MAXSTRING];
					familystring(str, *f);
					printf("%s splits into ", str);
					familystring(str, newf);
					printf("%s\n", str);*/

					clearfamily(&newf);

					return 1;
				}
			}
		}
	}
	addtolist(&unsolved, *f);
	//clearfamily(f);
	return 0;
}

void explore(family f, int side, int back)
{	if(back==0)
	{	for(int i=0; i<f.len; i++)
			if(f.numrepeats[i]>0)
			{	char str[MAXSTRING];
				familystring(str, f);
				//printf("first ");
				//printf(side ? "left " : "right ");
#ifdef PRINTEXPLORE
				printf("exploring %s as ", str);
#endif

				for(int j=0; j<f.numrepeats[i]; j++)
				{	family newf;
					familyinit(&newf);
					instancefamily(&newf, f, side, back);
					newf.digit[i+1] = f.repeats[i][j];
					if(examine(&newf))
						addtolist(&unsolved, newf);

#ifdef PRINTEXPLORE
					familystring(str, newf);
					printf("%s, ", str);
#endif

					clearfamily(&newf);
				}

				family copyf;
				familyinit(&copyf);
				copyfamily(&copyf, f);
				copyf.repeats[i] = NULL;
				copyf.numrepeats[i] = 0;
				if(examine(&copyf))
					addtolist(&unsolved, copyf);

#ifdef PRINTEXPLORE
				familystring(str, copyf);
				printf("%s\n", str);
#endif

				clearfamily(&copyf);

				break;
			}
	}
	else
	{	for(int i=f.len-1; i>=0; i--)
			if(f.numrepeats[i]>0)
			{	char str[MAXSTRING];
				familystring(str, f);
				//printf("last ");
				//printf(side ? "left " : "right ");
#ifdef PRINTEXPLORE
				printf("exploring %s as ", str);
#endif

				for(int j=0; j<f.numrepeats[i]; j++)
				{	family newf;
					familyinit(&newf);
					instancefamily(&newf, f, side, back);
					newf.digit[i+1] = f.repeats[i][j];
					if(examine(&newf))
						addtolist(&unsolved, newf);

#ifdef PRINTEXPLORE
					familystring(str, newf);
					printf("%s, ", str);
#endif

					clearfamily(&newf);
				}

				family copyf;
				familyinit(&copyf);
				copyfamily(&copyf, f);
				copyf.repeats[i] = NULL;
				copyf.numrepeats[i] = 0;
				if(examine(&copyf))
					addtolist(&unsolved, copyf);

#ifdef PRINTEXPLORE
				familystring(str, copyf);
				printf("%s\n", str);
#endif

				clearfamily(&copyf);

				break;
			}
	}
}

int main(int argc, char** argv)
{	prsize = 10000000;
	pr = malloc((prsize>>3)+1);
	memset(pr, 255, (prsize>>3)+1);
	for(int i=2;i*i<=prsize;i++) 
		if(pr[i>>3]&(1<<(i&7))) 
			for(int j=prsize/i;j>=i;j--) 
				if(pr[j>>3]&(1<<(j&7))) 
					pr[(i*j)>>3]&=~(1<<((i*j)&7));
	pr[0] &= 252;

	FILE* out = stdout;
	//FILE* out = fopen("basedata.txt", "w");
	//fclose(out);
	for(base=atoi(argv[1]); base<atoi(argv[2]); base++)
	{	//base = atoi(argv[1]);
		depth = atoi(argv[3]);

		//FILE* out = fopen("basedata.txt", "a");

		kernelinit();
		listinit(&unsolved);

		for(int i=0; i<base; i++)
			for(int j=0; j<base; j++)
				for(int k=0; k<base; k++)
				{	char str[4];
					if(i==0 && j==0)
						sprintf(str, "%c", digitchar(k));
					else if(i==0)
						sprintf(str, "%c%c", digitchar(j), digitchar(k));
					else
						sprintf(str, "%c%c%c", digitchar(i), digitchar(j), digitchar(k));
					if(newminimal(str))
					{	char* newstr = malloc(4);
						memcpy(newstr, str, 4);
						addtokernel(newstr);
					}
				}

		for(int i=1; i<base; i++)
			for(int j=0; j<base; j++)
			{	char* middles = calloc(base, sizeof(char));
				int middlesize = 0;
				for(int k=0; k<base; k++)
				{	char str[4];
					sprintf(str, "%c%c%c", digitchar(i), digitchar(k), digitchar(j));
					if(nosubword(str))
						middles[middlesize++] = k;
				}
				if(middlesize>0)
				{	family f;
					familyinit(&f);
					adddigit(&f, i, middles, middlesize);
					adddigit(&f, j, NULL, 0);
					if(!hasdivisor(f))
					{	//char tempstr[MAXSTRING];
						//familystring(tempstr, f);
						//printf("Exploring %s...\n", tempstr);
						explore(f, 1, 0);
					}
				}
				else
					free(middles);
			}

		list oldlist;
		listinit(&oldlist);
		for(int i=0; i<depth; i++)
		{	
			clearlist(&oldlist);
			oldlist = unsolved;
			listinit(&unsolved);

			int didsplit = 1;
			int splititer = 0;
			while(didsplit)
			{	didsplit = 0;
				for(int j=0; j<oldlist.size; j++)
					didsplit |= split(&(oldlist.fam[j]));

				clearlist(&oldlist);
				oldlist = unsolved;
				listinit(&unsolved);

				for(int j=0; j<oldlist.size; j++)
					if(examine(&(oldlist.fam[j])))
						addtolist(&unsolved, oldlist.fam[j]);

				clearlist(&oldlist);
				oldlist = unsolved;
				listinit(&unsolved);

				for(int j=0; j<oldlist.size; j++)
					didsplit |= split2(&(oldlist.fam[j]));

				clearlist(&oldlist);
				oldlist = unsolved;
				listinit(&unsolved);

				for(int j=0; j<oldlist.size; j++)
					if(examine(&(oldlist.fam[j])))
						addtolist(&unsolved, oldlist.fam[j]);

				clearlist(&oldlist);
				oldlist = unsolved;
				listinit(&unsolved);

				splititer++;
#ifdef PRINTSTATS
				printf("base %d\titeration %d\tsplit %d\tsize %d\tremain %d\n", base, i, splititer, K.size, oldlist.size);
#endif
			}

			//if(i==180)
			//	for(int j=0; j<oldlist.size; j++)
			//		explore(oldlist.fam[j], 2, (i/2)%2);
			//else
			for(int j=0; j<oldlist.size; j++)
				explore(oldlist.fam[j], i%2, (i/2)%2);
			//printf("base %d\titeration %d\tsize %d\tremain %d\n", base, i, K.size, unsolved.size);
#ifdef PRINTUNSOLVED
			for(int j=0; j<unsolved.size; j++)
			{	char str[MAXSTRING];
				familystring(str, unsolved.fam[j]);
				printf("%s\n", str);
			}
#endif
			/*char filename[100];
			sprintf(filename, "iter%d.txt", i);
			FILE* out = fopen(filename, "w");
			for(int j=0; j<unsolved.size; j++)
			{	char str[MAXSTRING];
				familystring(str, unsolved.fam[j]);
				fprintf(out, "%s\n", str);
			}
			fclose(out);*/
		}

		fprintf(out, "BASE %d:\n", base);

		kernel temp;
		temp.size = 0;
		temp.primes = NULL;
		for(int i=0; i<K.size; i++)
			if(nosubwordskip(K.primes[i], i))
			{	int size = ++temp.size;
				temp.primes = realloc(temp.primes, size*sizeof(char*));
				//temp.primes[size-1] = K.primes[i];
				temp.primes[size-1] = malloc(MAXSTRING);
				strcpy(temp.primes[size-1], K.primes[i]);
			}
		clearkernel();
		K = temp;
#ifdef PRINTKERNEL
		for(int i=0; i<K.size; i++)
			printf("%s\n", K.primes[i]);
#endif
		fprintf(out, "\tSize:\t%d\n", K.size);
		int width = strlen(K.primes[0]);
		for(int i=1; i<K.size; i++)
			if(width<strlen(K.primes[i]))
				width = strlen(K.primes[i]);
		fprintf(out, "\tWidth:\t%d\n", width);
		fprintf(out, "\tRemain:\t%d\n", unsolved.size);

		for(int i=0; i<unsolved.size; i++)
		{	char str[MAXSTRING];
			familystring(str, unsolved.fam[i]);
			fprintf(out, "%s\n", str);
		}

		clearkernel();
		clearlist(&unsolved);
		//fclose(out);
	}

	free(pr);
	return 0;
}
