#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>

int base;
int prsize;
char* pr;

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

void familyinit(family* p)
{	p->len = 0;
	p->numrepeats = NULL;
	p->digit = NULL;
	p->repeats = NULL;
}

void kernelinit(kernel* K)
{	K->size = 0;
	K->primes = NULL;
}

void addtokernel(kernel* K, char* p)
{	int size = ++K->size;
	K->primes = realloc(K->primes, size*sizeof(char*));
	K->primes[size-1] = p;
}

int nosubword(kernel K, char* p)
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

int newminimal(kernel K, char* p)
{	if(nosubword(K, p)==0)
		return 0;

	mpz_t temp;
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

void adddigit(family* f, char d, char* r, int n)
{	int len = ++f->len;
	f->digit = realloc(f->digit, len*sizeof(int));
	f->digit[len-1] = d;
	f->numrepeats = realloc(f->numrepeats, len*sizeof(char));
	f->numrepeats[len-1] = n;
	f->repeats = realloc(f->repeats, len*sizeof(char*));
	f->repeats[len-1] = r;
}

char digitchar(char digit)
{	if(digit>10)
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

int hasdivisor(family p)
{	mpz_t gcd, temp;
	mpz_init(gcd);
	mpz_init(temp);
	char str[100];
	emptyinstancestring(str, p);
	mpz_set_str(gcd, str, base);
	for(int i=0; i<p.len; i++)
		for(int j=0; j<p.numrepeats[i]; j++)
		{	instancestring(str, p, i, j);
			mpz_set_str(temp, str, base);
			mpz_gcd(gcd, gcd, temp);
		}

	if(mpz_cmp_ui(gcd, 1)>0)
	{	//familystring(str, p);
		//gmp_printf("%s has a divisor %Zd\n", str, gcd);
		return 1;
	}
	else
		return 0;

	mpz_clear(gcd);
	mpz_clear(temp);
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
	char* str;
	kernel K;
	kernelinit(&K);

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
				if(newminimal(K, str))
					addtokernel(&K, str);
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
				if(nosubword(K, str))
					middles[middlesize++] = k;
			}
			if(middlesize>0)
			{	family f;
				familyinit(&f);
				adddigit(&f, i, middles, middlesize);
				adddigit(&f, j, NULL, 0);
				if(!hasdivisor(f))
				{	char tempstr[100];
					familystring(tempstr, f);
					printf("Unsolved family: %s\n", tempstr);
				}
			}
			else
				free(middles);
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
