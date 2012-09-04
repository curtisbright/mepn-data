#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>

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

void addtolist(list* l, family f)
{	int size = ++l->size;
	l->fam = realloc(l->fam, size*sizeof(family));
	l->fam[size-1] = f;
}

void copylist(list* out, list in)
{	out->size = in.size;
	out->fam = malloc(in.size*sizeof(family));
	for(int i=0; i<in.size; i++)
		out->fam[i] = in.fam[i];
}

void familyinit(family* p)
{	p->len = 0;
	p->numrepeats = NULL;
	p->digit = NULL;
	p->repeats = NULL;
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

int isprime(char* p)
{	mpz_t temp;
	mpz_init(temp);
	mpz_set_str(temp, p, base);
	if(mpz_probab_prime_p(temp, 1, pr, prsize) > 0)
	{	mpz_clear(temp);
		return 1;
	}
	else
	{	mpz_clear(temp);
		return 0;
	}
}

int newminimal(char* p)
{	if(!nosubword(p))
		return 0;
	return isprime(p);
}

void adddigit(family* f, char d, char* r, int n)
{	int len = ++f->len;
	f->digit = realloc(f->digit, len*sizeof(int));
	f->digit[len-1] = d;
	f->numrepeats = realloc(f->numrepeats, len*sizeof(int));
	f->numrepeats[len-1] = n;
	f->repeats = realloc(f->repeats, len*sizeof(char*));
	f->repeats[len-1] = r;
}

char digitchar(char digit)
{	if(digit>=10)
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

int hasdivisor(family p)
{	mpz_t gcd, temp;
	mpz_init(gcd);
	mpz_init(temp);
	char str[100];
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
		return 0;

	if(mpz_cmp_ui(gcd, 1)>0)
	{	//familystring(str, p);
		//gmp_printf("%s has a divisor %Zd\n", str, gcd);
		return 1;
	}

	if(numrepeats<3)
	{	emptyinstancestring(str, p);
		mpz_set_str(gcd, str, base);
		for(int i=0; i<p.len; i++)
			for(int j=0; j<p.numrepeats[i]; j++)
				for(int k=0; k<p.len; k++)
					for(int l=0; l<p.numrepeats[k]; l++)
					{	doubleinstancestring(str, p, i, j, k, l);
						mpz_set_str(temp, str, base);
						mpz_gcd(gcd, gcd, temp);
					}

		if(mpz_cmp_ui(gcd, 1)==0)
			return 0;

		int gcdbeenset = 0;
		for(int i=0; i<p.len; i++)
			for(int j=0; j<p.numrepeats[i]; j++)
			{	instancestring(str, p, i, j);
				mpz_set_str(temp, str, base);
				if(gcdbeenset)
					mpz_gcd(gcd, gcd, temp);
				else
				{	gcdbeenset = 1;
					mpz_set(gcd, temp);
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
								mpz_gcd(gcd, gcd, temp);
							}
		if(mpz_cmp_ui(gcd, 1)>0)
			return 1;
	}

	return 0;

	mpz_clear(gcd);
	mpz_clear(temp);
}

void copyfamily(family* newf, family f)
{	for(int i=0; i<f.len; i++)
	{	char* repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
		memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
		adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
	}
}

void instancefamily(family* newf, family f, int side)
{	int firstrepeat = 1;
	for(int i=0; i<f.len; i++)
	{	char* repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
		memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
		if(f.numrepeats[i]>0 && firstrepeat)
		{	if(side)
			{	adddigit(newf, f.digit[i], NULL, 0);
				adddigit(newf, 0, repeatscopy, f.numrepeats[i]);
			}
			else
			{	adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
				adddigit(newf, 0, NULL, 0);
			}
			firstrepeat = 0;
		}
		else
			adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
	}
}

int examine(family* f)
{	char* str = malloc(100);
	emptyinstancestring(str, *f);
	if(!nosubword(str))
		return 0;
	else if(isprime(str))
	{	addtokernel(str);
		return 0;
	}
	free(str);

	int trivial = 1;
	for(int i=0; i<f->len; i++)
	{	int newnumrepeat = 0;
		for(int j=0; j<f->numrepeats[i]; j++)
		{	char tempstr[100];
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

int split(family f)
{	for(int i=0; i<f.len; i++)
	{	for(int j=0; j<f.numrepeats[i]; j++)
		{	char str[100];
			doubleinstancestring(str, f, i, j, i, j);
			if(nosubword(str) && isprime(str))
				addtokernel(str);
			
			return 0;	
		}
	}
	return 1;
}

void explore(family f, int side)
{	for(int i=0; i<f.len; i++)
		if(f.numrepeats[i]>0)
		{	

			for(int j=0; j<f.numrepeats[i]; j++)
			{	family newf;
				familyinit(&newf);
				instancefamily(&newf, f, side);
				newf.digit[i+1] = f.repeats[i][j];
				if(examine(&newf))
					addtolist(&unsolved, newf);
			}

			f.repeats[i] = NULL;
			f.numrepeats[i] = 0;
			if(examine(&f))
				addtolist(&unsolved, f);

			break;
		}
}

int main(int argc, char** argv)
{	prsize = 1000000;
	pr = malloc((prsize>>3)+1);
	memset(pr, 255, (prsize>>3)+1);
	for(int i=2;i*i<=prsize;i++) 
		if(pr[i>>3]&(1<<(i&7))) 
			for(int j=prsize/i;j>=i;j--) 
				if(pr[j>>3]&(1<<(j&7))) 
					pr[(i*j)>>3]&=~(1<<((i*j)&7)); 

	base = atoi(argv[1]);
	depth = atoi(argv[2]);
	char* str;
	kernelinit();
	listinit(&unsolved);

	for(int i=0; i<base; i++)
		for(int j=0; j<base; j++)
			for(int k=0; k<base; k++)
			{	str = calloc(4, sizeof(char));
				if(i==0 && j==0)
					sprintf(str, "%c", digitchar(k));
				else if(i==0)
					sprintf(str, "%c%c", digitchar(j), digitchar(k));
				else
					sprintf(str, "%c%c%c", digitchar(i), digitchar(j), digitchar(k));
				if(newminimal(str))
					addtokernel(str);
				else
					free(str);
			}

	for(int i=1; i<base; i++)
		for(int j=0; j<base; j++)
		{	char* middles = calloc(base, sizeof(char));
			int middlesize = 0;
			for(int k=0; k<base; k++)
			{	str = calloc(4, sizeof(char));
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
				{	//char tempstr[100];
					//familystring(tempstr, f);
					//printf("Exploring %s...\n", tempstr);
					explore(f, 1);
				}
			}
		}

	for(int i=0; i<depth; i++)
	{	list oldlist;
		copylist(&oldlist, unsolved);
		listinit(&unsolved);
		for(int j=0; j<oldlist.size; j++)
		{	//char tempstr[100];
			//familystring(tempstr, oldlist.fam[j]);
			//printf("Exploring %s...\n", tempstr);
			explore(oldlist.fam[j], i%2);
		}
	}

	if(unsolved.size>0)
		printf("Unsolved families:\n");
	for(int i=0; i<unsolved.size; i++)
	{	str = calloc(100, sizeof(char));
		familystring(str, unsolved.fam[i]);
		printf("%s\n", str);
		free(str);
	}

	printf("Prime kernel:\n");
	for(int i=0; i<K.size; i++)
		printf("%s\n", K.primes[i]);
	printf("Size:\t%d\n", K.size);
	int width = strlen(K.primes[0]);
	for(int i=1; i<K.size; i++)
		if(width<strlen(K.primes[i]))
			width = strlen(K.primes[i]);
	printf("Width:\t%d\n", width);

	return 0;
}
