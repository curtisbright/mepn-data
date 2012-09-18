compile:
	gcc kernel.c -o kernel -Ofast -lgmp -std=c99 -DPRINTSTATS
clean:
	rm kernel
