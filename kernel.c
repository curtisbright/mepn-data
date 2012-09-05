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

void addtolist(list* l, family f)
{	int size = ++l->size;
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
	f->digit = realloc(f->digit, len*sizeof(int));
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

void copyfamily(family* newf, family f)
{	for(int i=0; i<f.len; i++)
	{	char* repeatscopy = malloc(f.numrepeats[i]*sizeof(char));
		memcpy(repeatscopy, f.repeats[i], f.numrepeats[i]*sizeof(char));
		adddigit(newf, f.digit[i], repeatscopy, f.numrepeats[i]);
	}
}

int hasdivisor(family p)
{	mpz_t gcd, temp;
	mpz_init(gcd);
	mpz_init(temp);
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
	{	mpz_clear(gcd);
		mpz_clear(temp);
		return 0;
	}

	if(mpz_cmp_ui(gcd, 1)>0)
	{	//familystring(str, p);
		//gmp_printf("%s has a divisor %Zd\n", str, gcd);
		mpz_clear(gcd);
		mpz_clear(temp);
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
		{	mpz_clear(gcd);
			mpz_clear(temp);
			return 0;
		}

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
		{	mpz_clear(gcd);
			mpz_clear(temp);
			return 1;
		}
	}

	mpz_clear(gcd);
	mpz_clear(temp);
	return 0;
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

				clearfamily(&copyf);
				clearfamily(&newf);

				/*char str[MAXSTRING];
				familystring(str, copyf);
				printf("%s splits into ", str);
				familystring(str, copyf);
				printf("%s and ", str);
				familystring(str, newf);
				printf("%s\n", str);*/
			
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
					clearfamily(&copyf);

					//clearfamily(f);

					return 1;
				}
			}
		}
	}
	addtolist(&unsolved, *f);
	//clearfamily(f);
	return 0;
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
				//else
				clearfamily(&newf);
			}

			family copyf;
			familyinit(&copyf);
			copyfamily(&copyf, f);
			copyf.repeats[i] = NULL;
			copyf.numrepeats[i] = 0;
			if(examine(&copyf))
				addtolist(&unsolved, copyf);
			//else
			clearfamily(&copyf);

			break;
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

	FILE* out = fopen("basedata.txt", "w");
	fclose(out);
	//for(base=2; base<atoi(argv[1]); base++)
	{	base = atoi(argv[1]);
		depth = atoi(argv[2]);

		FILE* out = fopen("basedata.txt", "a");

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
						explore(f, 1);
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

			for(int j=0; j<oldlist.size; j++)
				split(&(oldlist.fam[j]));

			clearlist(&oldlist);
			oldlist = unsolved;
			listinit(&unsolved);

			for(int j=0; j<oldlist.size; j++)
				split2(&(oldlist.fam[j]));

			clearlist(&oldlist);
			oldlist = unsolved;
			listinit(&unsolved);

			for(int j=0; j<oldlist.size; j++)
				explore(oldlist.fam[j], i%2);

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
		//for(int i=0; i<K.size; i++)
		//	printf("%s\n", K.primes[i]);
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

		//clearkernel();
		clearlist(&unsolved);
		fclose(out);
	}

	free(pr);
	return 0;
}
