#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>
#define MAXSTRING 200

#ifdef PRINTALL
#define PRINTDATA
#define PRINTITER
#define PRINTSUMMARY
#define PRINTDIVISOR
#define PRINTDIVISORTWO
#define PRINTDIVISORTHREE
#define PRINTDIVISORFOUR
#define PRINTDIVISORFIVE
#define PRINTDIVISORSQUARE
#define PRINTDIVISORCUBE
#define PRINTSTATS
#define PRINTUNSOLVED
#define PRINTSPLIT
#define PRINTSPLITDOUBLE
#define PRINTSPLITTRIPLE
#define PRINTSPLITQUAD
#define PRINTSPLITQUINT
#define PRINTPRIMES
#define PRINTSUBWORD
#define PRINTEXPLORE
#define PRINTTRIVIAL
#define PRINTRESUME
#define PRINTDIVISOREXT
#define PRINTSPLITEXT
#endif

#if defined(PRINTDATA) || defined(PRINTITER)
#include <sys/stat.h>
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
int iter;
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
	int* listpos = malloc(n*sizeof(int));
	char* str = malloc(MAXSTRING*sizeof(char));
	if(issimple(unsolved->fam[0]))
		simplefamilystring(str, unsolved->fam[0]);
	else
		familystring(str, unsolved->fam[0]);
	strlist[0] = str;
	listpos[0] = 0;
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
		int inttemp;
		int intlast;
		for(int j=0; j<n; j++)
		{	if(addedtolist)
			{	temp = strlist[j];
				strlist[j] = last;
				last = temp;
				inttemp = listpos[j];
				listpos[j] = intlast;
				intlast = inttemp;
			}
			else if(strcmp(str,strlist[j])>0)
			{	addedtolist = 1;
				last = strlist[j];
				strlist[j] = str;
				intlast = listpos[j];
				listpos[j] = n;
				addtolist(&newlist, unsolved->fam[i], unsolved->split[i]);
			}
			else if(strcmp(str,strlist[j])==0)
			{	if(issimple(unsolved->fam[i]))
				{	char str1[MAXSTRING], str2[MAXSTRING];
					familystring(str1, unsolved->fam[i]);
					familystring(str2, newlist.fam[listpos[j]]);
					if(strlen(str1)<strlen(str2))
					{	clearfamily(&(newlist.fam[listpos[j]]));
						copyfamily(&(newlist.fam[listpos[j]]), unsolved->fam[i]);
					}
				}
				break;
			}
			else if(j==n-1)
			{	addedtolist = 1;
				last = str;
				intlast = n;
				addtolist(&newlist, unsolved->fam[i], unsolved->split[i]);
			}
		}
		if(addedtolist)
		{	n++;
			strlist = realloc(strlist, n*sizeof(char*));
			strlist[n-1] = last;
			listpos = realloc(listpos, n*sizeof(int));
			listpos[n-1] = intlast;
		}
		else
			free(str);
	}

	clearlist(unsolved);
	copylist(unsolved, newlist);
	clearlist(&newlist);

	for(int i=0; i<n; i++)
		free(strlist[i]);
	free(strlist);
	free(listpos);
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
	l->fam = realloc(l->fam, size*sizeof(family));
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
	if(mpz_probab_prime_p(temp, 25) > 0)
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

char invdigitchar(char input)
{	if(input>='0' && input<='9')
		return input-'0';
	else if(input>='A' && input<='Z')
		return input-'A'+10;
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
		if(p.digit[i]!=repeateddigit && (unsigned char)p.digit[i]!=255)
		{	j = i;
			break;
		}
	
	int k=p.len;
	for(int i=repeatedpos+1; i<p.len; i++)
		if(p.digit[i]!=repeateddigit && (unsigned char)p.digit[i]!=255)
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

void quintinstancestring(char* str, family p, int x1, int y1)
{	sprintf(str, "%c", 0);
	for(int i=0; i<p.len; i++)
	{	sprintf(str, "%s%c", str, digitchar(p.digit[i]));
		if(i==x1)
		{	sprintf(str, "%s%c%c%c%c%c%c%c%c%c%c", str, digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]), digitchar(p.repeats[x1][y1]));
		}
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
{	mpz_t gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty;
	mpz_inits(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
	char str[MAXSTRING];
	int numrepeats = 0;
	emptyinstancestring(str, p);
	mpz_set_str(empty, str, base);
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
		mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
		return 0;
	}

	if(mpz_cmp_ui(gcd, 1)>0 && mpz_cmp(empty, gcd)>0)
	{	
#ifdef PRINTDIVISOR
		familystring(str, p);
		gmp_printf("%s has a divisor %Zd\n", str, gcd);
#endif
		mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
		return 1;
	}

	for(int m=0; m<p.len; m++)
	{	if(p.numrepeats[m]==0)
			continue;

		emptyinstancestring(str, p);
		mpz_set_str(gcd1, str, base);

		for(int i=0; i<p.len; i++)
		{	if(i==m)
				continue;
			for(int j=0; j<p.numrepeats[i]; j++)
			{	instancestring(str, p, i, j);
				mpz_set_str(temp, str, base);
				mpz_gcd(gcd1, gcd1, temp);
			}
		}
		
		for(int i=0; i<p.numrepeats[m]; i++)
			for(int j=0; j<p.numrepeats[m]; j++)
			{	doubleinstancestring(str, p, m, i, m, j);
				mpz_set_str(temp, str, base);
				mpz_gcd(gcd1, gcd1, temp);
			}

		int gcdbeenset = 0;
		for(int n=0; n<p.numrepeats[m]; n++)
		{
			instancestring(str, p, m, n);
			mpz_set_str(temp, str, base);
			if(gcdbeenset)
				mpz_gcd(gcd2, gcd2, temp);
			else
			{	mpz_set(gcd2, temp);
				gcdbeenset = 1;
			}

			for(int i=0; i<p.len; i++)
			{	if(i==m)
					continue;
				for(int j=0; j<p.numrepeats[i]; j++)
				{	doubleinstancestring(str, p, i, j, m, n);
					mpz_set_str(temp, str, base);
					mpz_gcd(gcd2, gcd2, temp);
				}
			}
		
			for(int i=0; i<p.numrepeats[m]; i++)
				for(int j=0; j<p.numrepeats[m]; j++)
					for(int k=0; k<p.numrepeats[m]; k++)
					{	tripleinstancestring(str, p, m, i, m, j, m, k);
						mpz_set_str(temp, str, base);
						mpz_gcd(gcd2, gcd2, temp);
					}

		}

		if(mpz_cmp_ui(gcd1, 1)>0 && mpz_cmp_ui(gcd2, 1)>0 && mpz_cmp(empty, gcd1)>0 && mpz_cmp(empty, gcd2)>0)
		{	
#ifdef PRINTDIVISORTWO
			familystring(str, p);
			gmp_printf("%s has two divisors %Zd and %Zd\n", str, gcd1, gcd2);
#endif
			mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
			return 1;
		}

		mpz_set_ui(gcd1, 0);
		mpz_set_ui(gcd2, 0);
	}

	int i;
	int gcdbeenset = 0;
	for(i=0; i<p.len; i++)
	{	for(int j=0; j<p.numrepeats[i]; j++)
		{	instancestring(str, p, i, j);
			mpz_set_str(temp, str, base);
			if(gcdbeenset)
				mpz_gcd(gcd2, gcd2, temp);
			else
			{	gcdbeenset = 1;
				mpz_set(gcd2, temp);
			}
			for(int k=0; k<p.len; k++)
				for(int l=0; l<p.numrepeats[k]; l++)
					for(int n=0; n<p.numrepeats[k]; n++)
					{	tripleinstancestring(str, p, i, j, k, l, k, n);
						mpz_set_str(temp, str, base);
						mpz_gcd(gcd2, gcd2, temp);
					}
		}
		if(p.numrepeats[i]>0)
			break;
	}
	int firstrepeat = i;

	if(numrepeats==2)
	{	emptyinstancestring(str, p);
		mpz_set_str(gcd1, str, base);
		for(int i=0; i<p.len; i++)
			for(int j=0; j<p.numrepeats[i]; j++)
				for(int l=0; l<p.numrepeats[i]; l++)
				{	doubleinstancestring(str, p, i, j, i, l);
					mpz_set_str(temp, str, base);
					mpz_gcd(gcd1, gcd1, temp);
				}

		gcdbeenset = 0;
		for(i=firstrepeat+1; i<p.len; i++)
		{	for(int j=0; j<p.numrepeats[i]; j++)
			{	instancestring(str, p, i, j);
				mpz_set_str(temp, str, base);
				if(gcdbeenset)
					mpz_gcd(temp2, temp2, temp);
				else
				{	gcdbeenset = 1;
					mpz_set(temp2, temp);
				}
				for(int k=0; k<p.len; k++)
					for(int l=0; l<p.numrepeats[k]; l++)
						for(int n=0; n<p.numrepeats[k]; n++)
						{	tripleinstancestring(str, p, i, j, k, l, k, n);
							mpz_set_str(temp, str, base);
							mpz_gcd(temp2, temp2, temp);
						}
			}
			if(p.numrepeats[i]>0)
				break;
		}
		int secondrepeat = i;

		gcdbeenset = 0;
		for(int j=0; j<p.numrepeats[firstrepeat]; j++)
			for(int l=0; l<p.numrepeats[secondrepeat]; l++)
			{	doubleinstancestring(str, p, firstrepeat, j, secondrepeat, l);
				mpz_set_str(temp, str, base);
				if(gcdbeenset)
					mpz_gcd(temp3, temp3, temp);
				else
				{	gcdbeenset = 1;
					mpz_set(temp3, temp);
				}
			}

		for(i=0; i<p.numrepeats[firstrepeat]; i++)
			for(int j=0; j<p.numrepeats[firstrepeat]; j++)
				for(int k=0; k<p.numrepeats[firstrepeat]; k++)
					for(int l=0; l<p.numrepeats[secondrepeat]; l++)
					{	quadinstancestring(str, p, firstrepeat, i, firstrepeat, j, firstrepeat, k, secondrepeat, l);
						mpz_set_str(temp, str, base);
						mpz_gcd(temp3, temp3, temp);
					}

		for(i=0; i<p.numrepeats[firstrepeat]; i++)
			for(int j=0; j<p.numrepeats[secondrepeat]; j++)
				for(int k=0; k<p.numrepeats[secondrepeat]; k++)
					for(int l=0; l<p.numrepeats[secondrepeat]; l++)
					{	quadinstancestring(str, p, firstrepeat, i, secondrepeat, j, secondrepeat, k, secondrepeat, l);
						mpz_set_str(temp, str, base);
						mpz_gcd(temp3, temp3, temp);
					}

		if(mpz_cmp_ui(gcd1, 1)>0 && mpz_cmp_ui(gcd2, 1)>0 && mpz_cmp(empty, gcd1)>0 && mpz_cmp(empty, gcd2)>0 &&
			mpz_cmp_ui(temp2, 1)>0 && mpz_cmp_ui(temp3, 1)>0 && mpz_cmp(empty, temp2)>0 && mpz_cmp(empty, temp3)>0)
		{	
#ifdef PRINTDIVISORTWONEW
			familystring(str, p);
			gmp_printf("%s has four divisors %Zd, %Zd, %Zd, and %Zd\n", str, gcd1, gcd2, temp2, temp3);
#endif
			mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
			return 1;
		}
	}

	char end[MAXSTRING], start[MAXSTRING], middle[2];

	if(numrepeats==1)
	{	for(int i=0; i<p.len; i++)
		{	if(p.numrepeats[i]==1)
			{	endinstancestring(str, p, i);
				int zlen = strlen(str);
				mpz_set_str(z, str, base);
				mpz_set_ui(y, p.repeats[i][0]);
				startinstancestring(str, p, i);
				mpz_set_str(x, str, base);

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

				if(mpz_cmp_ui(temp, 1)>0 && mpz_cmp_ui(temp2, 1)>0 && mpz_cmp_ui(temp3, 1)>0 && mpz_cmp(empty, temp)>0 && mpz_cmp(empty, temp2)>0 && mpz_cmp(empty, temp3)>0)
				{	
#ifdef PRINTDIVISORTHREE
					familystring(str, p);
					gmp_printf("%s has three divisors %Zd, %Zd, and %Zd\n", str, temp, temp2, temp3);
#endif
					mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
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

				if(mpz_cmp_ui(temp, 1)>0 && mpz_cmp_ui(temp2, 1)>0 && mpz_cmp_ui(temp3, 1)>0 && mpz_cmp_ui(temp4, 1)>0 && mpz_cmp(empty, temp)>0 && mpz_cmp(empty, temp2)>0 && mpz_cmp(empty, temp3)>0 && mpz_cmp(empty, temp4)>0)
				{	
#ifdef PRINTDIVISORFOUR
					familystring(str, p);
					gmp_printf("%s has four divisors %Zd, %Zd, %Zd, and %Zd\n", str, temp, temp2, temp3, temp4);
#endif
					mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
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

				if(mpz_cmp_ui(temp, 1)>0 && mpz_cmp_ui(temp2, 1)>0 && mpz_cmp_ui(temp3, 1)>0 && mpz_cmp_ui(temp4, 1)>0 && mpz_cmp_ui(temp5, 1)>0 && mpz_cmp(empty, temp)>0 && mpz_cmp(empty, temp2)>0 && mpz_cmp(empty, temp3)>0 && mpz_cmp(empty, temp4)>0 && mpz_cmp(empty, temp5)>0)
				{	
#ifdef PRINTDIVISORFIVE
					familystring(str, p);
					gmp_printf("%s has five divisors %Zd, %Zd, %Zd, %Zd, and %Zd\n", str, temp, temp2, temp3, temp4, temp5);
#endif
					mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
					return 1;
				}

				mpz_gcd_ui(temp10, y, base-1);
				int g = mpz_get_ui(temp10);
				mpz_divexact_ui(temp, y, g);
				mpz_set(temp2, temp);
				mpz_addmul_ui(temp, x, (base-1)/g);
				mpz_ui_pow_ui(temp3, base, zlen);
				mpz_mul(temp, temp, temp3);
				mpz_mul(temp2, temp2, temp3);
				mpz_submul_ui(temp2, z, (base-1)/g);

				if(mpz_root(temp3, temp, 2)!=0 && mpz_sgn(temp2)>=0 && mpz_root(temp4, temp2, 2)!=0)
				{	mpz_sub(temp5, temp3, temp4);
					mpz_set_ui(temp6, base);
					if(mpz_cmp_ui(temp5, (base-1)/g)>0 && mpz_root(temp6, temp6, 2)!=0)
					{	
#ifdef PRINTDIVISORSQUARE
						familystring(str, p);
						gmp_printf("%s factors as a difference of squares\n", str);
						gmp_printf("%s(%s)^n%s = %Zd + %d^%d*%Zd*(%d^n-1)/%d + %d^(n+%d)*%Zd = (%Zd*%d^n-%Zd)/%d = (%Zd*%Zd^n-%Zd)*(%Zd*%Zd^n+%Zd)/%d\n", start, middle, end, z, base, zlen, y, base, base-1, base, zlen, x, temp, base, temp2, (base-1)/g, temp3, temp6, temp4, temp3, temp6, temp4, (base-1)/g);
#endif
						mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
						return 1;
					}
					else if(mpz_cmp_ui(temp5, (base-1)/g)>0 && mpz_cmp_ui(gcd2, 1)>0 && mpz_cmp(empty, gcd2)>0)
					{
#ifdef PRINTDIVISORSQUARE
						familystring(str, p);
						gmp_printf("%s factors as a difference of squares for even n, and has a factor %Zd for odd n\n", str, gcd2);
						gmp_printf("%s(%s)^n%s = %Zd + %d^%d*%Zd*(%d^n-1)/%d + %d^(n+%d)*%Zd = (%Zd*%d^n-%Zd)/%d = (%Zd*%d^(n/2)-%Zd)*(%Zd*%d^(n/2)+%Zd)/%d\n", start, middle, end, z, base, zlen, y, base, base-1, base, zlen, x, temp, base, temp2, (base-1)/g, temp3, base, temp4, temp3, base, temp4, (base-1)/g);
#endif
						mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
						return 1;
					}
				}

				if(mpz_root(temp3, temp, 3)!=0 && mpz_root(temp4, temp2, 3)!=0)
				{	mpz_sub(temp5, temp3, temp4);
					mpz_set_ui(temp6, base);
					if(mpz_cmp_ui(temp5, (base-1)/g)>0 && mpz_root(temp6, temp6, 3)!=0)
					{	
#ifdef PRINTDIVISORCUBE
						familystring(str, p);
						gmp_printf("%s factors as a difference of cubes\n", str);
						if(mpz_sgn(temp2)>=0)
							gmp_printf("%s(%s)^n%s = (%Zd*%d^n-%Zd)/%d = (%Zd*%Zd^n-%Zd)*((%Zd*%Zd^n)^2+%Zd*%Zd^n*%Zd+%Zd^2)/%d\n", start, middle, end, temp, base, temp2, (base-1)/g, temp3, temp6, temp4, temp3, temp6, temp3, temp6, temp4, temp4, (base-1)/g);
						else
							gmp_printf("%s(%s)^n%s = (%Zd*%d^n-(%Zd))/%d = (%Zd*%Zd^n-(%Zd))*((%Zd*%Zd^n)^2+%Zd*%Zd^n*(%Zd)+(%Zd)^2)/%d\n", start, middle, end, temp, base, temp2, (base-1)/g, temp3, temp6, temp4, temp3, temp6, temp3, temp6, temp4, temp4, (base-1)/g);
#endif
						mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
						return 1;
					}
				}
			}
		}
	}

	char residues[30] = {1};
	for(int i=0; i<p.len; i++)
	{	if((unsigned char)p.digit[i]!=255)
		{	char newresidues[30] = {0};
			for(int j=0; j<30; j++)
			{	if(residues[j]==1)
					newresidues[(j*base+p.digit[i])%30] = 1;
			}
			memcpy(residues, newresidues, 30);
		}
		int haschanged = 1;
		while(haschanged)
		{	haschanged = 0;
			for(int j=0; j<p.numrepeats[i]; j++)
			{	for(int l=0; l<30; l++)
				{	if(residues[l]==1 && residues[(l*base+p.repeats[i][j])%30]==0)
					{	residues[(l*base+p.repeats[i][j])%30] = 1;
						haschanged = 1;
					}
				}
			}
		}
	}

	int coprimeres = 0;
	for(int i=0; i<30; i++)
	{	if(residues[i]==1)
		{	mpz_set_ui(temp, i);
			mpz_gcd_ui(temp, temp, 30);
			if(mpz_cmp_ui(temp, 1)==0)
				coprimeres = 1;
		}
	}

	if(!coprimeres)
	{	
#ifdef PRINTDIVISOREXT
		familystring(str, p);
		gmp_printf("\nevery number in %s is divisible by one of 2, 3, or 5\n", str);
#endif
		mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
		return 1;
	}

	mpz_clears(gcd, temp, gcd1, gcd2, x, y, z, temp2, temp3, temp4, temp5, temp6, temp7, temp8, temp9, temp10, empty, NULL);
	return 0;
}

void instancefamily(family* newf, family f, int side, int pos)
{	for(int i=0; i<f.len; i++)
	{	char* repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
		memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
		if(i==pos)
		{	if(side==1)
			{	adddigit(newf, f.digit[i], NULL, 0);
				adddigit(newf, 0, repeatscopy, f.numrepeats[i]);
			}
			else if(side==0)
			{	adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
				adddigit(newf, 0, NULL, 0);
			}
		}
		else
			adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
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

	// simplify y*y^ny*
	char lastdigit = 0;
	int dosimplify = 0;
	for(int i=0; i<f->len; i++)
	{	if(dosimplify==1 && f->numrepeats[i]==1 && f->repeats[i][0]==lastdigit && (lastdigit==f->digit[i] || (unsigned char)f->digit[i]==255))
		{	f->repeats[i] = NULL;
			f->numrepeats[i] = 0;
		}
		if((unsigned char)f->digit[i]!=255)
		{	if(f->digit[i] != lastdigit)
				dosimplify = 0;
			lastdigit = f->digit[i];
		}
		if(f->numrepeats[i]==1)
		{	dosimplify = 1;
			lastdigit = f->repeats[i][0];
		}
		else if(f->numrepeats[i]>1)
			dosimplify = 0;
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

int split(family* f, list* unsolved, char insplit)
{	if(insplit==0)
	{	addtolist(unsolved, *f, 0);
		return 0;
	}
	for(int i=0; i<f->len; i++)
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

				addtolist(unsolved, copyf, 2);
				addtolist(unsolved, newf, 2);

#ifdef PRINTSPLITDOUBLE
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

				addtolist(unsolved, copyf, 2);
				addtolist(unsolved, newf, 2);
				addtolist(unsolved, newf2, 2);

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

				addtolist(unsolved, copyf, 2);
				addtolist(unsolved, newf, 2);
				addtolist(unsolved, newf2, 2);
				addtolist(unsolved, newf3, 2);

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

			quintinstancestring(str, *f, i, j);
	
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

				addtolist(unsolved, copyf, 2);

				for(int l=1; l<=9; l++)
				{
					family newf;
					familyinit(&newf);
					for(int k=0; k<copyf.len; k++)
					{	char* newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
						memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
						adddigit(&newf, copyf.digit[k], newrepeats, copyf.numrepeats[k]);
						if(k==i)
						{	for(int m=0; m<l; m++)
							{	newrepeats = malloc(copyf.numrepeats[k]*sizeof(char));
								memcpy(newrepeats, copyf.repeats[k], copyf.numrepeats[k]*sizeof(char));
								adddigit(&newf, removeddigit, newrepeats, copyf.numrepeats[k]);
							}
						}
					}

					addtolist(unsolved, newf, 2);
					clearfamily(&newf);

				}

				clearfamily(&copyf);

#ifdef PRINTSPLITQUINT
				char str[MAXSTRING];
				familystring(str, *f);
				printf("%s splits ten ways\n", str);
#endif
			
				return 1;
			}

			if(iter>5)
			{	mpz_t gcd, empty, temp;
				mpz_inits(gcd, empty, temp, NULL);
				emptyinstancestring(str, *f);
				mpz_set_str(empty, str, base);
				mpz_set_str(gcd, str, base);
				for(int ii=0; ii<f->len; ii++)
				{	for(int jj=0; jj<f->numrepeats[ii]; jj++)
					{	instancestring(str, *f, ii, jj);
						mpz_set_str(temp, str, base);
						if(i!=ii || j!=jj)
							mpz_gcd(gcd, gcd, temp);
					}
				}

				if(mpz_cmp_ui(gcd, 1)>0 && mpz_cmp(empty, gcd)>0)
				{	mpz_clears(gcd, empty, temp, NULL);

					family copyf;
					familyinit(&copyf);
					for(int ii=0; ii<f->len; ii++)
					{	char* repeatscopy = malloc(f->numrepeats[ii]*sizeof(char));
						memcpy(repeatscopy, f->repeats[ii], f->numrepeats[ii]*sizeof(char));
						adddigit(&copyf, f->digit[ii], repeatscopy, f->numrepeats[ii]);
						if(i==ii)
						{	repeatscopy = malloc(f->numrepeats[ii]*sizeof(char));
							memcpy(repeatscopy, f->repeats[ii], f->numrepeats[ii]*sizeof(char));
							adddigit(&copyf, f->repeats[i][j], repeatscopy, f->numrepeats[i]);
						}
					}
					addtolist(unsolved, copyf, 2);
#ifdef PRINTSPLITEXT
					familystring(str, *f);
					printf("%s splits into ", str);
					familystring(str, copyf);
					printf("%s\n", str);
#endif
					clearfamily(&copyf);
					return 1;
				}
			}

		}
	}
	addtolist(unsolved, *f, 1);
	return 0;
}

int split2(family* f, list* unsolved, char insplit)
{	if(insplit==0)
	{	addtolist(unsolved, *f, 0);
		return 0;
	}
	for(int i=0; i<f->len; i++)
	{	for(int j=0; j<f->numrepeats[i]; j++)
		{	for(int m=i; m<f->len; m++)
			{	for(int k=0; k<f->numrepeats[m]; k++)
				{	if(m==i && j<=k)
						continue;
					char str1[MAXSTRING];
					char str2[MAXSTRING];
					char str3[MAXSTRING];
					char str4[MAXSTRING];
					char str5[MAXSTRING];
					char str6[MAXSTRING];
					doubleinstancestring(str1, *f, i, j, m, k);
					tripleinstancestring(str3, *f, i, j, m, k, i, j);
					tripleinstancestring(str4, *f, i, k, m, j, i, k);
					quadinstancestring(str5, *f, i, j, m, k, i, j, m, k);
					quadinstancestring(str6, *f, i, k, m, j, i, k, m, j);
					if(m==i)
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
						printf("%s [because of %s, %s]\n", str, str1, str2);
#endif

						clearfamily(&copyf);

						return 1;
					}
					else if(m==i && iter>5 && (!nosubword(str1)))
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
						printf("%s [because of %s]\n", str, str1);
#endif

						clearfamily(&newf);

						return 1;
					}
					else if(m==i && iter>5 && (!nosubword(str2)))
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
						printf("%s [because of %s]\n", str, str2);
#endif

						clearfamily(&newf);

						return 1;
					}
					else if(m==i && iter>5 && (!nosubword(str3)))
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

								newrepeats = malloc(f->numrepeats[l]*sizeof(char));
								memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
								adddigit(&newf, 255, newrepeats, f->numrepeats[l]);

								newnumrepeats = 0;
								removeddigit = f->repeats[i][j];
								for(int m=0; m<f->numrepeats[i]; m++)
								{	if(m!=j)
										newf.repeats[i+2][newnumrepeats++] = f->repeats[i][m];
								}
								newf.numrepeats[i+2] = newnumrepeats;
							}
						}
						addtolist(unsolved, newf, 1);

#ifdef PRINTSPLITEXT
						char str[MAXSTRING];
						familystring(str, *f);
						printf("%s splits into ", str);
						familystring(str, newf);
						printf("%s [because of %s]\n", str, str3);
#endif

						clearfamily(&newf);

						return 1;
					}
					else if(m==i && iter>5 && (!nosubword(str4)))
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

								newrepeats = malloc(f->numrepeats[l]*sizeof(char));
								memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
								adddigit(&newf, 255, newrepeats, f->numrepeats[l]);

								newnumrepeats = 0;
								removeddigit = f->repeats[i][k];
								for(int m=0; m<f->numrepeats[i]; m++)
								{	if(m!=k)
										newf.repeats[i+2][newnumrepeats++] = f->repeats[i][m];
								}
								newf.numrepeats[i+2] = newnumrepeats;
							}
						}
						addtolist(unsolved, newf, 1);

#ifdef PRINTSPLITEXT
						char str[MAXSTRING];
						familystring(str, *f);
						printf("%s splits into ", str);
						familystring(str, newf);
						printf("%s [because of %s]\n", str, str4);
#endif

						clearfamily(&newf);

						return 1;
					}
					else if(m==i && iter>5 && (!nosubword(str5)))
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

								newrepeats = malloc(f->numrepeats[l]*sizeof(char));
								memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
								adddigit(&newf, 255, newrepeats, f->numrepeats[l]);

								newnumrepeats = 0;
								removeddigit = f->repeats[i][j];
								for(int m=0; m<f->numrepeats[i]; m++)
								{	if(m!=j)
										newf.repeats[i+2][newnumrepeats++] = f->repeats[i][m];
								}
								newf.numrepeats[i+2] = newnumrepeats;

								newrepeats = malloc(f->numrepeats[l]*sizeof(char));
								memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
								adddigit(&newf, 255, newrepeats, f->numrepeats[l]);

								newnumrepeats = 0;
								removeddigit = f->repeats[i][k];
								for(int m=0; m<f->numrepeats[i]; m++)
								{	if(m!=k)
										newf.repeats[i+3][newnumrepeats++] = f->repeats[i][m];
								}
								newf.numrepeats[i+3] = newnumrepeats;
							}
						}
						addtolist(unsolved, newf, 1);

#ifdef PRINTSPLITEXT
						char str[MAXSTRING];
						familystring(str, *f);
						printf("%s splits into ", str);
						familystring(str, newf);
						printf("%s [because of %s]\n", str, str5);
#endif

						clearfamily(&newf);

						return 1;
					}
					else if(m==i && iter>5 && (!nosubword(str6)))
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

								newrepeats = malloc(f->numrepeats[l]*sizeof(char));
								memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
								adddigit(&newf, 255, newrepeats, f->numrepeats[l]);

								newnumrepeats = 0;
								removeddigit = f->repeats[i][k];
								for(int m=0; m<f->numrepeats[i]; m++)
								{	if(m!=k)
										newf.repeats[i+2][newnumrepeats++] = f->repeats[i][m];
								}
								newf.numrepeats[i+2] = newnumrepeats;

								newrepeats = malloc(f->numrepeats[l]*sizeof(char));
								memcpy(newrepeats, f->repeats[l], f->numrepeats[l]*sizeof(char));
								adddigit(&newf, 255, newrepeats, f->numrepeats[l]);

								newnumrepeats = 0;
								removeddigit = f->repeats[i][j];
								for(int m=0; m<f->numrepeats[i]; m++)
								{	if(m!=j)
										newf.repeats[i+3][newnumrepeats++] = f->repeats[i][m];
								}
								newf.numrepeats[i+3] = newnumrepeats;
							}
						}
						addtolist(unsolved, newf, 1);

#ifdef PRINTSPLITEXT
						char str[MAXSTRING];
						familystring(str, *f);
						printf("%s splits into ", str);
						familystring(str, newf);
						printf("%s [because of %s]\n", str, str6);
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
						for(int l=0; l<copyf.numrepeats[m]; l++)
						{	if(l!=k)
								copyf.repeats[m][newnumrepeats++] = copyf.repeats[m][l];
						}
						copyf.numrepeats[m] = newnumrepeats;
						addtolist(unsolved, copyf, 1);

#ifdef PRINTSPLIT
						familystring(str, copyf);
						printf("%s [because of %s]\n", str, str1);
#endif

						clearfamily(&copyf);

						return 1;
					}
				}
			}
		}
	}
	addtolist(unsolved, *f, insplit-1);
	return 0;
}

void explore(family f, int side, int pos, list* unsolved)
{	int count = 0;
	for(int i=0; i<f.len; i++)
		if(f.numrepeats[i]>0)
			count++;
	pos = pos % count;
	count = 0;
	for(int i=0; i<f.len; i++)
	{	if(f.numrepeats[i]>0)
		{	if(pos==count)
			{	char str[MAXSTRING];
				familystring(str, f);
#ifdef PRINTEXPLORE
				printf("exploring %s as ", str);
#endif

				for(int j=0; j<f.numrepeats[i]; j++)
				{	family newf;
					familyinit(&newf);
					instancefamily(&newf, f, side, i);
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
			count++;
		}
	}
}

int main(int argc, char** argv)
{	char filename[100];
	sprintf(filename, "summary.txt");
	FILE* summaryfile;
#ifdef CLEARSUMMARY
	summaryfile = fopen(filename, "w");
	fclose(summaryfile);
#endif

	family f;
	familyinit(&f);

	int l, h, resume = 0;
	if(argc==1)
	{	printf("Computes minimal primes for bases between l and h,\n");
		printf("possibly along with a set of unsolved families.\n");
		printf("Usage: minimal l h\n");
		printf("To resume base b from iter i: minimal resume b i\n");
		return 0;
	}
	else if(strcmp(argv[1], "resume")==0)
	{	l = h = atoi(argv[2]);
		resume = 1;
		iter = atoi(argv[3]);
	}
	else if(argc==2)
		l = h = atoi(argv[1]);
	else
	{	l = atoi(argv[1]);
		h = atoi(argv[2]);
	}

#ifdef PRINTDATA
	mkdir("data", S_IRWXU);
#endif

#ifdef PRINTITER
	mkdir("iter", S_IRWXU);
#endif

	for(base=l; base<=h; base++)
	{	
#ifdef PRINTSTATS
		printf("base %d...\n", base);
#endif
		kernelinit();
		list unsolved;
		listinit(&unsolved);

		if(!resume)
		{	for(int i=0; i<base; i++)
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
						{	explore(f, 1, 0, &unsolved);
						}
					}
					else
						free(middles);
				}
			iter = 0;
		}
		else
		{	char str[100];
			sprintf(str, "iter/minimal-base%d-iter%d.txt", base, iter);
			FILE* in = fopen(str, "r");
			char line[MAXSTRING];
			while(fgets(line, MAXSTRING, in)!=NULL)
			{	line[strlen(line)-1] = '\0';
				char* newstr = malloc(strlen(line)+1);
				strcpy(newstr, line);
				addtokernel(newstr);
#ifdef PRINTRESUME
				printf("added %s to kernel\n", line);
#endif
			}
			fclose(in);
			sprintf(str, "iter/unsolved-base%d-iter%d.txt", base, iter);
			FILE* out = fopen(str, "r");
			while(fgets(line, MAXSTRING, in)!=NULL)
			{	family f;
				familyinit(&f);
				for(int i=0; i<strlen(line)-1; i++)
				{	int digit;
					if(line[i]=='{')
						digit = 255;
					else
						digit = invdigitchar(line[i]);
					if(line[i]!='{' && line[i+1]!='{')
					{	adddigit(&f, digit, NULL, 0);
					}
					else
					{	int k = strchr(line+i+1, '}')-(line+i+1)+(line[i]=='{'?1:0)-1;
						char* middles = calloc(k, sizeof(char));
						for(int j=i+2-(line[i]=='{'?1:0); j<k+i+2-(line[i]=='{'?1:0); j++)
						{	middles[j-(i+2-(line[i]=='{'?1:0))] = invdigitchar(line[j]);
						}
						adddigit(&f, digit, middles, k);
						i = k+i+2-(line[i]=='{'?1:0)+1;
					}
				}
				addtolist(&unsolved, f, 2);
#ifdef PRINTRESUME
				familystring(str, f);
				printf("added %s to unknown list\n", str);
#endif
				clearfamily(&f);
			}
			fclose(out);
			iter++;
		}

		for(;; iter++)
		{	if(!onlysimple(unsolved))
			{	int didsplit = 1;
				int splititer = 0;
				while(didsplit)
				{	didsplit = 0;

					list oldlist;
					copylist(&oldlist, unsolved);
					clearlist(&unsolved);

					for(int j=0; j<oldlist.size; j++)
						didsplit |= split(&(oldlist.fam[j]), &unsolved, oldlist.split[j]);

					clearlist(&oldlist);
					removedupes(&unsolved);
					copylist(&oldlist, unsolved);
					clearlist(&unsolved);

					for(int j=0; j<oldlist.size; j++)
						if(oldlist.split[j]==0 || examine(&(oldlist.fam[j])))
							addtolist(&unsolved, oldlist.fam[j], oldlist.split[j]);

					clearlist(&oldlist);
					removedupes(&unsolved);
					copylist(&oldlist, unsolved);
					clearlist(&unsolved);

					for(int j=0; j<oldlist.size; j++)
						didsplit |= split2(&(oldlist.fam[j]), &unsolved, oldlist.split[j]);

					clearlist(&oldlist);
					removedupes(&unsolved);
					copylist(&oldlist, unsolved);
					clearlist(&unsolved);

					for(int j=0; j<oldlist.size; j++)
						if(oldlist.split[j]==0 || examine(&(oldlist.fam[j])))
							addtolist(&unsolved, oldlist.fam[j], oldlist.split[j]);				

					clearlist(&oldlist);
					removedupes(&unsolved);

					splititer++;
#ifdef PRINTSTATS
					printf("base %d\titeration %d\tsplit %d\tsize %d\tremain %d\n", base, iter, splititer, K.size, unsolved.size);
#endif
				}
			}
			else
				break;

			list oldlist;
			copylist(&oldlist, unsolved);
			clearlist(&unsolved);

			for(int j=0; j<oldlist.size; j++)
				explore(oldlist.fam[j], iter%2, iter, &unsolved);

			clearlist(&oldlist);
			removedupes(&unsolved);

#ifdef PRINTUNSOLVED
			printf("Unsolved families after explore:\n");
			printlist(unsolved);
#endif

#ifdef PRINTITER
			char filename[100];
			sprintf(filename, "iter/unsolved-base%d-iter%d.txt", base, iter);
			FILE* out = fopen(filename, "w");
			for(int j=0; j<unsolved.size; j++)
			{	char str[MAXSTRING];
				familystring(str, unsolved.fam[j]);
				fprintf(out, "%s\n", str);
			}
			fclose(out);

			filename[100];
			sprintf(filename, "iter/minimal-base%d-iter%d.txt", base, iter);
			out = fopen(filename, "w");
			for(int j=0; j<K.size; j++)
			{	fprintf(out, "%s\n", K.primes[j]);
			}
			fclose(out);
#endif

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
				temp.primes[size-1] = malloc(strlen(K.primes[i])+1);
				strcpy(temp.primes[size-1], K.primes[i]);
			}
		clearkernel();
		K = temp;

#ifdef PRINTDATA
		sprintf(filename, "data/minimal.%d.txt", base);
		FILE* kernelfile = fopen(filename, "w");
		for(int i=0; i<K.size; i++)
			fprintf(kernelfile, "%s\n", K.primes[i]);
		fclose(kernelfile);
#endif

#ifdef PRINTSUMMARY
		sprintf(filename, "summary.txt");
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
#endif

#ifdef PRINTDATA
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
#endif

		clearkernel();
		clearlist(&unsolved);
	}

	free(pr);
	return 0;
}
