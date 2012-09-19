#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>
#define MAXSTRING 20000

#ifdef PRINTALL
#define PRINTDIVISOR
#define PRINTDIVISORSPECIAL
#define PRINTDIVISORFOUR
#define PRINTDIVISORFIVE
#define PRINTSTATS
#define PRINTUNSOLVED
#define PRINTSPLIT
#define PRINTSPLITSPECIAL
#define PRINTSPLITTRIPLE
#define PRINTSPLITQUAD
#define PRINTSPLITQUINT
#define PRINTPRIMES
#define PRINTSUBWORD
#define PRINTEXPLORE
#define PRINTTRIVIAL
#define PRINTSIMPLE
#endif

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
	char* split;
} list;

void familystring(char* str, family p);
void clearfamily(family* f);
void copyfamily(family* newf, family f);
void adddigit(family* f, char d, char* r, int n);
void familyinit(family* p);
void addtolist(list* l, family f, char split);
void simplefamilystring(char* str, family p);
int issimple(family f);

int base;
int depth;
kernel K;
int prsize;
char* pr;

void listinit(list* l)
{	l->size = 0;
	l->fam = NULL;
	l->split = NULL;
}

void copylist(list* out, list in)
{	out->size = in.size;
	out->fam = malloc(in.size*sizeof(family));
	out->split = malloc(in.size*sizeof(char));
	for(int i=0; i<in.size; i++)
	{	familyinit(&(out->fam[i]));
		copyfamily(&(out->fam[i]), in.fam[i]);
		out->split[i] = in.split[i];
	}	
}

void clearlist(list* l)
{	for(int i=0; i<l->size; i++)
		clearfamily(&(l->fam[i]));
	free(l->fam);
	free(l->split);
	listinit(l);
}

void removedupes(list* unsolved)
{	if(unsolved->size==0)
		return;
	list newlist;
	listinit(&newlist);
	int n = 1;
	char** strlist = malloc(n*sizeof(char*));
	char* str = malloc(MAXSTRING*sizeof(char));
	if(issimple(unsolved->fam[0]))
		simplefamilystring(str, unsolved->fam[0]);
	else
		familystring(str, unsolved->fam[0]);
	strlist[0] = str;
	addtolist(&newlist, unsolved->fam[0], unsolved->split[0]);
	for(int i=1; i<unsolved->size; i++)
	{	str = malloc(MAXSTRING*sizeof(char));
		if(issimple(unsolved->fam[i]))
			simplefamilystring(str, unsolved->fam[i]);
		else
			familystring(str, unsolved->fam[i]);
		int addedtolist = 0;
		char* temp;
		char* last;
		for(int j=0; j<n; j++)
		{	if(addedtolist)
			{	temp = strlist[j];
				strlist[j] = last;
				last = temp;
			}
			else if(strcmp(str,strlist[j])>0)
			{	addedtolist = 1;
				last = strlist[j];
				strlist[j] = str;
				addtolist(&newlist, unsolved->fam[i], unsolved->split[i]);
			}
			else if(strcmp(str,strlist[j])==0)
				break;
			else if(j==n-1)
			{	addedtolist = 1;
				last = str;
				addtolist(&newlist, unsolved->fam[i], unsolved->split[i]);
			}
		}
		if(addedtolist)
		{	n++;
			strlist = realloc(strlist, n*sizeof(char*));
			strlist[n-1] = last;
		}
		else
			free(str);
	}

#if 0
	printf("Distinct unsolved list:\n");
	for(int i=0; i<n; i++)
		printf("%s\n", strlist[i]);
#endif

	clearlist(unsolved);
	copylist(unsolved, newlist);

	for(int i=0; i<n; i++)
		free(strlist[i]);
	free(strlist);
}

int issimple(family f)
{	int hasrepeat = 0;
	for(int i=0; i<f.len; i++)
	{	if(f.numrepeats[i]>1)
			return 0;
		if(f.numrepeats[i]==1)
		{	if(hasrepeat)
				return 0;
			hasrepeat = 1;
		}
	}
	return (hasrepeat==1);
}

int onlysimple(list l)
{	for(int i=0; i<l.size; i++)
	{	if(!issimple(l.fam[i]))
			return 0;
	}
	return 1;
}

void printlist(list l)
{	for(int i=0; i<l.size; i++)
	{	char str[MAXSTRING];
		familystring(str, l.fam[i]);
		printf("%s\n", str);
	}
}

void simpleprintlist(list l)
{	for(int i=0; i<l.size; i++)
	{	char str[MAXSTRING];
		simplefamilystring(str, l.fam[i]);
		printf("%s\n", str);
	}
}

void addtolist(list* l, family f, char split)
{	int size = ++l->size;
	l->fam = (family*)realloc(l->fam, size*sizeof(family));
	familyinit(&((l->fam)[size-1]));
	copyfamily(&((l->fam)[size-1]), f);
	l->split = realloc(l->split, size*sizeof(char));
	l->split[size-1] = split;
}

void kernelinit()
{	K.size = 0;
	K.primes = NULL;
}

void addtokernel(char* p)
{	int size = ++K.size;
	K.primes = realloc(K.primes, size*sizeof(char*));
	K.primes[size-1] = p;
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

void simplefamilystring(char* str, family p)
{	sprintf(str, "%c", 0);
	char repeateddigit;
	int repeatedpos;
	for(int i=0; i<p.len; i++)
		if(p.numrepeats[i]==1)
		{	repeateddigit = p.repeats[i][0];
			repeatedpos = i;
			break;
		}
	
	int j=-1;
	for(int i=repeatedpos; i>=0; i--)
		if(p.digit[i]!=repeateddigit && p.digit[i]!=255)
		{	j = i;
			break;
		}
	
	int k=p.len;
	for(int i=repeatedpos+1; i<p.len; i++)
		if(p.digit[i]!=repeateddigit && p.digit[i]!=255)
		{	k = i;
			break;
		}

	for(int i=0; i<=j; i++)
		sprintf(str, "%s%c", str, digitchar(p.digit[i]));
	sprintf(str, "%s%c*", str, digitchar(repeateddigit));
	for(int i=k; i<p.len; i++)
		sprintf(str, "%s%c", str, digitchar(p.digit[i]));
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

void quadinstancestring(char* str, family p, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{	sprintf(str, "%c", 0);
	for(int i=0; i<p.len; i++)
	{	sprintf(str, "%s%c", str, digitchar(p.digit[i]));
		if(i==x1)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x1][y1]));
		if(i==x2)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x2][y2]));
		if(i==x3)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x3][y3]));
		if(i==x4)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x4][y4]));
	}
}

void quintinstancestring(char* str, family p, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5)
{	sprintf(str, "%c", 0);
	for(int i=0; i<p.len; i++)
	{	sprintf(str, "%s%c", str, digitchar(p.digit[i]));
		if(i==x1)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x1][y1]));
		if(i==x2)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x2][y2]));
		if(i==x3)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x3][y3]));
		if(i==x4)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x4][y4]));
		if(i==x5)
			sprintf(str, "%s%c", str, digitchar(p.repeats[x5][y5]));
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
{	mpz_t gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10;
	mpz_inits(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
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
		mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
		return 0;
	}

	if(mpz_cmp_ui(gcd, 1)>0)
	{	
#ifdef PRINTDIVISOR
		familystring(str, p);
		gmp_printf("%s has a divisor %Zd\n", str, gcd);
#endif
		mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
		return 1;
	}

	if(numrepeats==2)
	{	int firstrepeatpos, lastrepeatpos;
		for(int i=0; i<p.len; i++)
			if(p.numrepeats[i]>0)
			{	firstrepeatpos = i;
				break;
			}
		for(int i=p.len-1; i>=0; i--)
			if(p.numrepeats[i]>0)
			{	lastrepeatpos = i;
				break;
			}

		emptyinstancestring(str, p);
		mpz_set_str(gcd1, str, base);
		for(int i=0; i<p.numrepeats[firstrepeatpos]; i++)
		{	instancestring(str, p, firstrepeatpos, i);
			mpz_set_str(temp, str, base);
			mpz_gcd(gcd1, gcd1, temp);
		}

		for(int i=0; i<p.numrepeats[lastrepeatpos]; i++)
			for(int j=0; j<p.numrepeats[lastrepeatpos]; j++)
			{	doubleinstancestring(str, p, lastrepeatpos, i, lastrepeatpos, j);
				mpz_set_str(temp, str, base);
				mpz_gcd(gcd1, gcd1, temp);
			}

		instancestring(str, p, lastrepeatpos, 0);
		mpz_set_str(gcd2, str, base);
		for(int i=1; i<p.numrepeats[lastrepeatpos]; i++)
		{	instancestring(str, p, lastrepeatpos, i);
			mpz_set_str(temp, str, base);
			mpz_gcd(gcd2, gcd2, temp);
		}

		for(int i=0; i<p.numrepeats[lastrepeatpos]; i++)
			for(int j=0; j<p.numrepeats[firstrepeatpos]; j++)
			{	doubleinstancestring(str, p, lastrepeatpos, i, firstrepeatpos, j);
				mpz_set_str(temp, str, base);
				mpz_gcd(gcd2, gcd2, temp);
			}

		for(int i=0; i<p.numrepeats[lastrepeatpos]; i++)
			for(int j=0; j<p.numrepeats[lastrepeatpos]; j++)
				for(int k=0; k<p.numrepeats[lastrepeatpos]; k++)
				{	tripleinstancestring(str, p, lastrepeatpos, i, lastrepeatpos, j, lastrepeatpos, k);
					mpz_set_str(temp, str, base);
					mpz_gcd(gcd2, gcd2, temp);
				}

		if(mpz_cmp_ui(gcd1, 1)>0 && mpz_cmp_ui(gcd2, 1)>0)
		{	
#ifdef PRINTDIVISORSPECIAL
			familystring(str, p);
			gmp_printf("%s has two divisors %Zd and %Zd\n", str, gcd1, gcd2);
#endif
			mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
			return 1;
		}

		emptyinstancestring(str, p);
		mpz_set_str(gcd1, str, base);
		for(int i=0; i<p.numrepeats[lastrepeatpos]; i++)
		{	instancestring(str, p, lastrepeatpos, i);
			mpz_set_str(temp, str, base);
			mpz_gcd(gcd1, gcd1, temp);
		}

		for(int i=0; i<p.numrepeats[firstrepeatpos]; i++)
			for(int j=0; j<p.numrepeats[firstrepeatpos]; j++)
			{	doubleinstancestring(str, p, firstrepeatpos, i, firstrepeatpos, j);
				mpz_set_str(temp, str, base);
				mpz_gcd(gcd1, gcd1, temp);
			}

		instancestring(str, p, firstrepeatpos, 0);
		mpz_set_str(gcd2, str, base);
		for(int i=1; i<p.numrepeats[firstrepeatpos]; i++)
		{	instancestring(str, p, firstrepeatpos, i);
			mpz_set_str(temp, str, base);
			mpz_gcd(gcd2, gcd2, temp);
		}

		for(int i=0; i<p.numrepeats[firstrepeatpos]; i++)
			for(int j=0; j<p.numrepeats[lastrepeatpos]; j++)
			{	doubleinstancestring(str, p, firstrepeatpos, i, lastrepeatpos, j);
				mpz_set_str(temp, str, base);
				mpz_gcd(gcd2, gcd2, temp);
			}

		for(int i=0; i<p.numrepeats[firstrepeatpos]; i++)
			for(int j=0; j<p.numrepeats[firstrepeatpos]; j++)
				for(int k=0; k<p.numrepeats[firstrepeatpos]; k++)
				{	tripleinstancestring(str, p, firstrepeatpos, i, firstrepeatpos, j, firstrepeatpos, k);
					mpz_set_str(temp, str, base);
					mpz_gcd(gcd2, gcd2, temp);
				}

		if(mpz_cmp_ui(gcd1, 1)>0 && mpz_cmp_ui(gcd2, 1)>0)
		{	
#ifdef PRINTDIVISORSPECIAL
			familystring(str, p);
			gmp_printf("%s has two divisors %Zd and %Zd\n", str, gcd1, gcd2);
#endif
			mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
			return 1;
		}

		mpz_set_ui(gcd1, 0);
		mpz_set_ui(gcd2, 0);
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
			mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
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
				{	
#ifdef PRINTDIVISOR
					familystring(str, p);
					gmp_printf("%s has three divisors %Zd, %Zd, and %Zd\n", str, temp, temp2, temp3);
#endif
					mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
					return 1;
				}

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
				sprintf(str, "%s%s%s%s%s%s%s%s\n", start, middle, middle, middle, middle, middle, middle, end);
				mpz_set_str(temp7, str, base);
				sprintf(str, "%s%s%s%s%s%s%s%s%s\n", start, middle, middle, middle, middle, middle, middle, middle, end);
				mpz_set_str(temp8, str, base);

				mpz_gcd(temp, temp, temp5);
				mpz_gcd(temp2, temp2, temp6);
				mpz_gcd(temp3, temp3, temp7);
				mpz_gcd(temp4, temp4, temp8);

				if(mpz_cmp_ui(temp, 1)>0 && mpz_cmp_ui(temp2, 1)>0 && mpz_cmp_ui(temp3, 1)>0 && mpz_cmp_ui(temp4, 1)>0)
				{	
#ifdef PRINTDIVISORFOUR
					familystring(str, p);
					gmp_printf("%s has four divisors %Zd, %Zd, %Zd, and %Zd\n", str, temp, temp2, temp3, temp4);
#endif
					mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
					return 1;
				}

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
				sprintf(str, "%s%s%s%s%s%s%s%s\n", start, middle, middle, middle, middle, middle, middle, end);
				mpz_set_str(temp7, str, base);
				sprintf(str, "%s%s%s%s%s%s%s%s%s\n", start, middle, middle, middle, middle, middle, middle, middle, end);
				mpz_set_str(temp8, str, base);
				sprintf(str, "%s%s%s%s%s%s%s%s%s%s\n", start, middle, middle, middle, middle, middle, middle, middle, middle, end);
				mpz_set_str(temp9, str, base);
				sprintf(str, "%s%s%s%s%s%s%s%s%s%s%s\n", start, middle, middle, middle, middle, middle, middle, middle, middle, middle, end);
				mpz_set_str(temp10, str, base);

				mpz_gcd(temp, temp, temp6);
				mpz_gcd(temp2, temp2, temp7);
				mpz_gcd(temp3, temp3, temp8);
				mpz_gcd(temp4, temp4, temp9);
				mpz_gcd(temp5, temp5, temp10);

				if(mpz_cmp_ui(temp, 1)>0 && mpz_cmp_ui(temp2, 1)>0 && mpz_cmp_ui(temp3, 1)>0 && mpz_cmp_ui(temp4, 1)>0 && mpz_cmp_ui(temp5, 1)>0)
				{	
#ifdef PRINTDIVISORFIVE
					familystring(str, p);
					gmp_printf("%s has five divisors %Zd, %Zd, %Zd, %Zd, and %Zd\n", str, temp, temp2, temp3, temp4, temp5);
#endif
					mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
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
						mpz_set_ui(temp7, base);
						if(mpz_cmp_ui(temp5, 1)>0 && mpz_cmp_ui(temp6, 1)>0 && mpz_root(temp7, temp7, 2)!=0)
						{	
#ifdef PRINTDIVISOR
							familystring(str, p);
							gmp_printf("%s factors as a difference of squares\n", str);
							gmp_printf("%s(%s)^n%s = %Zd + %d^%d*%Zd*(%d^n-1)/%d + %d^(n+%d)*%Zd = (%Zd*%d^n-%Zd)/%d = (%Zd*%d^(n/2)-%Zd)*(%Zd*%d^(n/2)+%Zd)/%d\n", start, middle, end, z, base, zlen, y, base, base-1, base, zlen, x, temp, base, temp2, base-1, temp3, base, temp4, temp3, base, temp4, base-1);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
							return 1;
						}
						else if(mpz_cmp_ui(temp5, 1)>0 && mpz_cmp_ui(temp6, 1)>0 && mpz_cmp_ui(gcd2, 1)>0)
						{
#ifdef PRINTDIVISOR
							familystring(str, p);
							gmp_printf("%s factors as a difference of squares for even n, and has a factor %Zd for odd n\n", str, gcd2);
							gmp_printf("%s(%s)^n%s = %Zd + %d^%d*%Zd*(%d^n-1)/%d + %d^(n+%d)*%Zd = (%Zd*%d^n-%Zd)/%d = (%Zd*%d^(n/2)-%Zd)*(%Zd*%d^(n/2)+%Zd)/%d\n", start, middle, end, z, base, zlen, y, base, base-1, base, zlen, x, temp, base, temp2, base-1, temp3, base, temp4, temp3, base, temp4, base-1);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
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
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
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
							gmp_printf("%s(%s)^n%s = %Zd + %d^%d*%Zd*(%d^n-1)/%d + %d^(n+%d)*%Zd = (%Zd*%d^n-%Zd)/%d = (%Zd*%d^(n/2)-%Zd)*(%Zd*%d^(n/2)+%Zd)/%d\n", start, middle, end, z, base, zlen, y, base, base-1, base, zlen, x, temp, base, temp2, base-1, temp3, base, temp4, temp3, base, temp4, base-1);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, NULL);
							return 1;
						}
						else if(mpz_cmp_ui(temp5, base-1)>0 && mpz_cmp_ui(temp6, base-1)>0 && mpz_cmp_ui(gcd2, 1)>0)
						{
#ifdef PRINTDIVISOR
							familystring(str, p);
							gmp_printf("%s factors as a difference of squares for even n, and has a factor %Zd for odd n\n", str, gcd2);
							gmp_printf("%s(%s)^n%s = %Zd + %d^%d*%Zd*(%d^n-1)/%d + %d^(n+%d)*%Zd = (%Zd*%d^n-%Zd)/%d = (%Zd*%d^(n/2)-%Zd)*(%Zd*%d^(n/2)+%Zd)/%d\n", start, middle, end, z, base, zlen, y, base, base-1, base, zlen, x, temp, base, temp2, base-1, temp3, base, temp4, temp3, base, temp4, base-1);
#endif
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
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
							mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
							return 1;
						}
					}
				}
			}
	}

	mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, NULL);
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
	char tempstr[MAXSTRING];
	emptyinstancestring(str, *f);
	if(!nosubword(str))
	{	free(str);
#ifdef PRINTSUBWORD
		familystring(tempstr, *f);
		printf("%s has a subword in kernel\n", tempstr);
#endif
		return 0;
	}
	else if(isprime(str))
	{	addtokernel(str);
#ifdef PRINTPRIMES
		familystring(tempstr, *f);
		printf("%s has a prime\n", tempstr);
#endif
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
	{	
#ifdef PRINTTRIVIAL
		familystring(tempstr, *f);
		printf("%s is trivial\n", tempstr);
#endif
		return 0;
	}

	if(hasdivisor(*f))
	{	return 0;
	}

	return 1;
}

int split(family* f, list* unsolved)
{	for(int i=0; i<f->len; i++)
	{	for(int j=0; j<f->numrepeats[i]; j++)
		{	char str[MAXSTRING];
			doubleinstancestring(str, *f, i, j, i, j);
			
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

				addtolist(unsolved, copyf, 1);
				addtolist(unsolved, newf, 1);

#ifdef PRINTSPLIT
				char str[MAXSTRING];
				familystring(str, *f);
				printf("%s splits into ", str);
				familystring(str, copyf);
				printf("%s and ", str);
				familystring(str, newf);
				printf("%s\n", str);
#endif

				clearfamily(&copyf);
				clearfamily(&newf);
			
				return 1;
			}

			tripleinstancestring(str, *f, i, j, i, j, i, j);

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

				family newf2;
				familyinit(&newf2);
				for(int k=0; k<copyf.len; k++)
				{	char* newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
					memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
					adddigit(&newf2, copyf.digit[k], newrepeats, copyf.numrepeats[k]);
					if(k==i)
					{	newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf2, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf2, removeddigit, newrepeats, copyf.numrepeats[k]);
					}
				}

				addtolist(unsolved, copyf, 1);
				addtolist(unsolved, newf, 1);
				addtolist(unsolved, newf2, 1);

#ifdef PRINTSPLITTRIPLE
				char str[MAXSTRING];
				familystring(str, *f);
				printf("%s splits into ", str);
				familystring(str, copyf);
				printf("%s and ", str);
				familystring(str, newf);
				printf("%s and ", str);
				familystring(str, newf2);
				printf("%s\n", str);
#endif

				clearfamily(&copyf);
				clearfamily(&newf);
				clearfamily(&newf2);
			
				return 1;
			}

			quadinstancestring(str, *f, i, j, i, j, i, j, i, j);
	
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

				family newf2;
				familyinit(&newf2);
				for(int k=0; k<copyf.len; k++)
				{	char* newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
					memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
					adddigit(&newf2, copyf.digit[k], newrepeats, copyf.numrepeats[k]);
					if(k==i)
					{	newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf2, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf2, removeddigit, newrepeats, copyf.numrepeats[k]);
					}
				}

				family newf3;
				familyinit(&newf3);
				for(int k=0; k<copyf.len; k++)
				{	char* newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
					memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
					adddigit(&newf3, copyf.digit[k], newrepeats, copyf.numrepeats[k]);
					if(k==i)
					{	newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf3, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf3, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf3, removeddigit, newrepeats, copyf.numrepeats[k]);
					}
				}

				addtolist(unsolved, copyf, 1);
				addtolist(unsolved, newf, 1);
				addtolist(unsolved, newf2, 1);
				addtolist(unsolved, newf3, 1);

#ifdef PRINTSPLITQUAD
				char str[MAXSTRING];
				familystring(str, *f);
				printf("%s splits into ", str);
				familystring(str, copyf);
				printf("%s and ", str);
				familystring(str, newf);
				printf("%s and ", str);
				familystring(str, newf2);
				printf("%s and ", str);
				familystring(str, newf3);
				printf("%s\n", str);
#endif

				clearfamily(&copyf);
				clearfamily(&newf);
				clearfamily(&newf2);
				clearfamily(&newf3);
			
				return 1;
			}

			quintinstancestring(str, *f, i, j, i, j, i, j, i, j, i, j);
	
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

				family newf2;
				familyinit(&newf2);
				for(int k=0; k<copyf.len; k++)
				{	char* newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
					memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
					adddigit(&newf2, copyf.digit[k], newrepeats, copyf.numrepeats[k]);
					if(k==i)
					{	newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf2, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf2, removeddigit, newrepeats, copyf.numrepeats[k]);
					}
				}

				family newf3;
				familyinit(&newf3);
				for(int k=0; k<copyf.len; k++)
				{	char* newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
					memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
					adddigit(&newf3, copyf.digit[k], newrepeats, copyf.numrepeats[k]);
					if(k==i)
					{	newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf3, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf3, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf3, removeddigit, newrepeats, copyf.numrepeats[k]);
					}
				}

				family newf4;
				familyinit(&newf4);
				for(int k=0; k<copyf.len; k++)
				{	char* newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
					memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
					adddigit(&newf4, copyf.digit[k], newrepeats, copyf.numrepeats[k]);
					if(k==i)
					{	newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf4, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf4, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf4, removeddigit, newrepeats, copyf.numrepeats[k]);
						newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf4, removeddigit, newrepeats, copyf.numrepeats[k]);
					}
				}

				addtolist(unsolved, copyf, 1);
				addtolist(unsolved, newf, 1);
				addtolist(unsolved, newf2, 1);
				addtolist(unsolved, newf3, 1);
				addtolist(unsolved, newf4, 1);

#ifdef PRINTSPLITQUINT
				char str[MAXSTRING];
				familystring(str, *f);
				printf("%s splits into ", str);
				familystring(str, copyf);
				printf("%s and ", str);
				familystring(str, newf);
				printf("%s and ", str);
				familystring(str, newf2);
				printf("%s and ", str);
				familystring(str, newf3);
				printf("%s and ", str);
				familystring(str, newf4);
				printf("%s\n", str);
#endif

				clearfamily(&copyf);
				clearfamily(&newf);
				clearfamily(&newf2);
				clearfamily(&newf3);
				clearfamily(&newf4);
			
				return 1;
			}

		}
	}
	addtolist(unsolved, *f, 0);
	return 0;
}

int split2(family* f, list* unsolved)
{	for(int i=0; i<f->len; i++)
	{	for(int j=0; j<f->numrepeats[i]; j++)
		{	for(int m=i; m<f->len; m++)
			{	for(int k=0; k<f->numrepeats[m]; k++)
				{	if(m==i && j<=k)
						continue;
					char str1[MAXSTRING];
					char str2[MAXSTRING];
					doubleinstancestring(str1, *f, i, j, m, k);
					doubleinstancestring(str2, *f, i, k, m, j);
					if(m==i && !nosubword(str1) && !nosubword(str2))
					{	family copyf;
						familyinit(&copyf);
						copyfamily(&copyf, *f);
						int newnumrepeats = 0;
						for(int l=0; l<copyf.numrepeats[i]; l++)
						{	if(l!=j)
								copyf.repeats[i][newnumrepeats++] = copyf.repeats[i][l];
						}
						copyf.numrepeats[i] = newnumrepeats;
						addtolist(unsolved, copyf, 1);

	#ifdef PRINTSPLIT
						char str[MAXSTRING];
						familystring(str, *f);
						printf("%s splits into ", str);
						familystring(str, copyf);
						printf("%s and ", str);
	#endif

						clearfamily(&copyf);

						familyinit(&copyf);
						copyfamily(&copyf, *f);
						newnumrepeats = 0;
						for(int l=0; l<copyf.numrepeats[i]; l++)
						{	if(l!=k)
								copyf.repeats[i][newnumrepeats++] = copyf.repeats[i][l];
						}
						copyf.numrepeats[i] = newnumrepeats;
						addtolist(unsolved, copyf, 1);

	#ifdef PRINTSPLIT
						familystring(str, copyf);
						printf("%s\n", str);
	#endif

						clearfamily(&copyf);

						return 1;
					}
					else if(m==i && (!nosubword(str1)) && (f->numrepeats[i])<=2)
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
						addtolist(unsolved, newf, 1);

	#ifdef PRINTSPLIT
						char str[MAXSTRING];
						familystring(str, *f);
						printf("%s splits into ", str);
						familystring(str, newf);
						printf("%s\n", str);
	#endif

						clearfamily(&newf);

						return 1;
					}
					else if(m==i && (!nosubword(str2)) && (f->numrepeats[i])<=2)
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
						addtolist(unsolved, newf, 1);

	#ifdef PRINTSPLIT
						char str[MAXSTRING];
						familystring(str, *f);
						printf("%s splits into ", str);
						familystring(str, newf);
						printf("%s\n", str);
	#endif

						clearfamily(&newf);

						return 1;
					}
					else if(m>i && !nosubword(str1))
					{	family copyf;
						familyinit(&copyf);
						copyfamily(&copyf, *f);
						int newnumrepeats = 0;
						for(int l=0; l<copyf.numrepeats[i]; l++)
						{	if(l!=j)
								copyf.repeats[i][newnumrepeats++] = copyf.repeats[i][l];
						}
						copyf.numrepeats[i] = newnumrepeats;
						addtolist(unsolved, copyf, 1);

	#ifdef PRINTSPLITSPECIAL
						char str[MAXSTRING];
						familystring(str, *f);
						printf("%s splits into ", str);
						familystring(str, copyf);
						printf("%s and ", str);
	#endif

						clearfamily(&copyf);

						familyinit(&copyf);
						copyfamily(&copyf, *f);
						newnumrepeats = 0;
						for(int l=0; l<copyf.numrepeats[m]; l++)
						{	if(l!=k)
								copyf.repeats[m][newnumrepeats++] = copyf.repeats[m][l];
						}
						copyf.numrepeats[m] = newnumrepeats;
						addtolist(unsolved, copyf, 1);

	#ifdef PRINTSPLITSPECIAL
						familystring(str, copyf);
						printf("%s\n", str);
	#endif

						clearfamily(&copyf);

						return 1;
					}
				}
			}
		}
	}
	addtolist(unsolved, *f, 0);
	return 0;
}

void explore(family f, int side, int back, list* unsolved)
{	if(back==0)
	{	for(int i=0; i<f.len; i++)
			if(f.numrepeats[i]>0)
			{	char str[MAXSTRING];
				familystring(str, f);
#ifdef PRINTEXPLORE
				printf("exploring %s as ", str);
#endif

				for(int j=0; j<f.numrepeats[i]; j++)
				{	family newf;
					familyinit(&newf);
					instancefamily(&newf, f, side, back);
					newf.digit[i+1] = f.repeats[i][j];
					if(examine(&newf))
						addtolist(unsolved, newf, 1);

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
					addtolist(unsolved, copyf, 1);

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
#ifdef PRINTEXPLORE
				printf("exploring %s as ", str);
#endif

				for(int j=0; j<f.numrepeats[i]; j++)
				{	family newf;
					familyinit(&newf);
					instancefamily(&newf, f, side, back);
					newf.digit[i+1] = f.repeats[i][j];
					if(examine(&newf))
						addtolist(unsolved, newf, 1);

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
					addtolist(unsolved, copyf, 1);

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
	for(base=atoi(argv[1]); base<=atoi(argv[2]); base++)
	{	kernelinit();
		list unsolved;
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
						explore(f, 1, 0, &unsolved);
					}
				}
				else
					free(middles);
			}

		for(int i=0; /*i<depth*/; i++)
		{	if(!onlysimple(unsolved))
			{	int didsplit = 1;
				int splititer = 0;
				while(didsplit)
				{	didsplit = 0;

					list oldlist;
					copylist(&oldlist, unsolved);
					clearlist(&unsolved);

					for(int j=0; j<oldlist.size; j++)
						didsplit |= split(&(oldlist.fam[j]), &unsolved);

					clearlist(&oldlist);
					removedupes(&unsolved);
					copylist(&oldlist, unsolved);
					clearlist(&unsolved);

					for(int j=0; j<oldlist.size; j++)
						if(examine(&(oldlist.fam[j])))
							addtolist(&unsolved, oldlist.fam[j], oldlist.split[j]);

					clearlist(&oldlist);
					removedupes(&unsolved);
					copylist(&oldlist, unsolved);
					clearlist(&unsolved);

					for(int j=0; j<oldlist.size; j++)
						didsplit |= split2(&(oldlist.fam[j]), &unsolved);

					clearlist(&oldlist);
					removedupes(&unsolved);
					copylist(&oldlist, unsolved);
					clearlist(&unsolved);

					for(int j=0; j<oldlist.size; j++)
					{	if(examine(&(oldlist.fam[j])))
							addtolist(&unsolved, oldlist.fam[j], oldlist.split[j]);
					}

					clearlist(&oldlist);
					removedupes(&unsolved);

					splititer++;
#ifdef PRINTSTATS
					printf("base %d\titeration %d\tsplit %d\tsize %d\tremain %d\n", base, i, splititer, K.size, unsolved.size);
#endif
				}
			}
			else
				break;

			list oldlist;
			copylist(&oldlist, unsolved);
			clearlist(&unsolved);

			for(int j=0; j<oldlist.size; j++)
				explore(oldlist.fam[j], i%2, (i/2)%2, &unsolved);

			clearlist(&oldlist);
			removedupes(&unsolved);

#ifdef PRINTUNSOLVED
			printf("Unsolved families after explore:\n");
			printlist(unsolved);
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

			if(unsolved.size==0)
				break;
		}

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

		char filename[100];
		sprintf(filename, "data/kernel.%d.txt", base);
		FILE* kernelfile = fopen(filename, "w");
		for(int i=0; i<K.size; i++)
			fprintf(kernelfile, "%s\n", K.primes[i]);
		fclose(kernelfile);

		sprintf(filename, "summary.txt", base);
		FILE* summaryfile = fopen(filename, "w");
		fclose(summaryfile);
		summaryfile = fopen(filename, "a");
		fprintf(summaryfile, "BASE %d:\n", base);
		fprintf(summaryfile, "\tSize:\t%d\n", K.size);
		int width = strlen(K.primes[0]);
		for(int i=1; i<K.size; i++)
			if(width<strlen(K.primes[i]))
				width = strlen(K.primes[i]);
		fprintf(summaryfile, "\tWidth:\t%d\n", width);
		fprintf(summaryfile, "\tRemain:\t%d\n", unsolved.size);
		fclose(summaryfile);

		sprintf(filename, "data/unsolved.%d.txt", base);
		FILE* unsolvedfile = fopen(filename, "w");
		for(int i=0; i<unsolved.size; i++)
		{	char str[MAXSTRING];
			if(issimple(unsolved.fam[i]))
				simplefamilystring(str, unsolved.fam[i]);
			else
				familystring(str, unsolved.fam[i]);
			fprintf(unsolvedfile, "%s\n", str);
		}
		fclose(unsolvedfile);

		clearkernel();
		clearlist(&unsolved);
	}

	free(pr);
	return 0;
}
