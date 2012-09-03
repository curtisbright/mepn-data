#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <string.h>

typedef struct
{	int base;
	int len;
	int* numrepeats;
	char* digit;
	char** repeats;
} family;

void init(family* p, int b)
{	p->base = b;
	p->len = 0;
	p->numrepeats = calloc(10, sizeof(int));
	p->digit = calloc(10, sizeof(char));
	p->repeats = calloc(10, sizeof(char*));
	for(int i=0; i<10; i++)
		p->repeats[i] = calloc(b, sizeof(char));
}

void adddigit(family* p, char d, char* r, int n)
{	int len = p->len++;
	p->digit[len] = d;
	p->numrepeats[len] = n;
	p->repeats[len] = r;
}

void printdigit(char digit)
{	if(digit>10)
		printf("%c", digit+'A'-10);
	else
		printf("%c", digit+'0');
}

void printfamily(family p)
{	for(int i=0; i<p.len; i++)
	{	printdigit(p.digit[i]);
		if(p.numrepeats[i]>0)
		{	printf("{");
			for(int j=0; j<p.numrepeats[i]; j++)
				printdigit(p.repeats[i][j]);
			printf("}*");
		}
	}
}

int main(int argc, char** argv)
{	family p;
	char a[3] = {0, 4, 6};
	init(&p, 10);
	adddigit(&p, 6, NULL, 0);
	adddigit(&p, 0, a, 3);
	adddigit(&p, 9, NULL, 0);

	printfamily(p);
	printf("\n");

	return 0;
}
