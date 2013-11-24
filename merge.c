#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	for(int base=2; base<29; base++)
	{	char filenamein1[100], filenamein2[100], filenameout[100];
		sprintf(filenamein1, "data/sieve.%d.out.txt", base);
		FILE* in1 = fopen(filenamein1, "r");
		if(in1==NULL)
			continue;
		sprintf(filenamein2, "srsieve/sieve.%d.out.txt", base);
		FILE* in2 = fopen(filenamein2, "r");
		if(in2==NULL)
		{	fclose(in1);
			continue;
		}
		sprintf(filenameout, "srsieve/tmp-sieve.%d.out.txt", base);
		FILE* out = fopen(filenameout, "w");
		int min, cur;
		char line1[100], line2[100];
		while(fgets(line2, 100, in2)!=NULL)
		{	if(strchr(line2, '*')!=NULL)
			{	min = -1;
				rewind(in1);
				while(fgets(line1, 100, in1)!=NULL)
				{	if(strcmp(line1, line2)==0)
					{	fprintf(out, "%s", line2);
						fgets(line1, 100, in1);
						if(line1!=NULL && strchr(line1, '*')==NULL)
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

		remove(filenamein1);
		remove(filenamein2);
		rename(filenameout, filenamein1);
		execl("cp", "cp", filenamein1, filenamein2);
	}

	return 0;
}
