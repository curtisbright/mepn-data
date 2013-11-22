#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	for(int base=2; base<29; base++)
	{	char filename[100], filename2[100];
		sprintf(filename2, "data/sieve.%d.out.txt", base);
		FILE* in1 = fopen(filename2, "r");
		sprintf(filename, "srsieve/sieve.%d.out.txt", base);
		FILE* in2 = fopen(filename, "r");
		if(in1==NULL || in2==NULL)
			continue;
		sprintf(filename, "srsieve/tmp-sieve.%d.out.txt", base);
		FILE* out = fopen(filename, "w");
		int min, cur;
		char line1[100], line2[100];
		fgets(line2, 100, in2);
		fprintf(out, "%s", line2);
		while(fgets(line2, 100, in2)!=NULL)
		{	if(strchr(line2, '*')!=NULL)
			{	min = -1;
				rewind(in1);
				while(fgets(line1, 100, in1)!=NULL)
				{	if(strcmp(line1, line2)==0)
					{	fprintf(out, "%s", line2);
						fgets(line1, 100, in1);
						min = atoi(line1);
						break;
					}
				}
			}
			else if(min!=-1)
			{	cur = atoi(line2);
				if(cur>=min)
					fprintf(out, "%s", line2);
			}
		}

		fclose(in1);
		fclose(in2);
		fclose(out);

		remove(filename2);
		rename(filename, filename2);
	}

	return 0;
}
