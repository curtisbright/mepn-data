#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <gmp.h>

int main(int argc, char** argv)
{

	DIR *dp;
	struct dirent *ep;

	mpz_t p;
	mpz_init(p);

	dp = opendir("./");
	int count=0;
	if(dp != NULL)
	{	while(ep = readdir(dp))
		{	char filename[100];
			strcpy(filename, ep->d_name);
			filename[8] = '\0';
			if(strcmp(filename, "unsolved")==0)
			{	strcpy(strchr(filename+9, '.'), "\0");
				int n = atoi(filename+9);
				int base = n;
				if(n>28)
					continue;
				//printf("base %d:\n", n);
				FILE* in = fopen(ep->d_name, "r");
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
					mpz_set_str(x, start, n);
					mpz_set_str(y, middle, n);
					mpz_set_str(z, end, n);
					mpz_gcd_ui(temp10, y, base-1);
					int g = mpz_get_ui(temp10);
					mpz_divexact_ui(temp, y, g);
					mpz_set(temp2, temp);
					mpz_addmul_ui(temp, x, (base-1)/g);
					mpz_ui_pow_ui(temp3, base, zlen);
					mpz_mul(temp, temp, temp3);
					mpz_mul(temp2, temp2, temp3);
					mpz_submul_ui(temp2, z, (base-1)/g);
					//gmp_printf("%d - base: %d x: %Zd y: %Zd z: %Zd X: %Zd Y: %Zd\n", count, base, x, y, z, temp, temp2);
					gmp_printf("%s(%s)^n%s = (%Zd*%d^n-(%Zd))/%d\n", start, middle, end, temp, base, temp2, (base-1)/g);
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

	return 0;
}
