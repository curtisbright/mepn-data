#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	for(int base=2; base<29; base++)
	{	char filenamein1[100], filenamein2[100], filenameout[100];
		sprintf(filenamein1, "data/sieve.%d.txt", base);
		FILE* in1 = fopen(filenamein1, "r");
		if(in1==NULL)
			continue;
		sprintf(filenamein2, "srsieve/sieve.%d.txt", base);
		FILE* in2 = fopen(filenamein2, "r");
		if(in2==NULL)
		{	fclose(in1);
			continue;
		}
		sprintf(filenameout, "srsieve/tmp-sieve.%d.txt", base);
		FILE* out = fopen(filenameout, "w");
		int n1, n2;
		char line1[100], line2[100];
		fgets(line2, 100, in2);
		fprintf(out, "%s", line2);
		while(fgets(line2, 100, in2)!=NULL)
		{	if(strchr(line2, '*')!=NULL)
			{	long pos = ftell(in2);
				rewind(in1);
				while(fgets(line1, 100, in1)!=NULL)
				{	if(strcmp(line1, line2)==0)
					{	fprintf(out, "%s", line2);
						if(fgets(line1, 100, in1)==NULL)
							break;
						pos = ftell(in2);
						if(fgets(line2, 100, in2)==NULL)
							break;
						while(1)
						{	if(strchr(line1, '*')==NULL)
								n1 = atoi(line1);
							else
								break;
							if(strchr(line2, '*')==NULL)
								n2 = atoi(line2);
							else
								break;
							if(n1==n2)
							{	fprintf(out, "%s", line2);
								if(fgets(line1, 100, in1)==NULL)
									break;
								pos = ftell(in2);
								if(fgets(line2, 100, in2)==NULL)
									break;
							}
							else if(n1>n2)
							{	pos = ftell(in2);
								if(fgets(line2, 100, in2)==NULL)
									break;
							}
							else if(n1<n2)
								if(fgets(line1, 100, in1)==NULL)
									break;
						}
						break;
					}
				}
				fseek(in2, pos, SEEK_SET);
			}
		}

		fclose(in1);
		fclose(in2);
		fclose(out);

		remove(filenamein1);
		remove(filenamein2);
		rename(filenameout, filenamein1);
		char copy[100];
		sprintf(copy, "cp %s %s", filenamein1, filenamein2);
		system(copy);
	}

	return 0;
}
