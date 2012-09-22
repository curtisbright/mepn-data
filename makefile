compile:
	gcc kernel.c -o kernel -Ofast -lgmp -std=c99 -DPRINTSTATS
debug:
	gcc kernel.c -o kernel -Ofast -lgmp -std=c99 -DPRINTSTATS -g -pg
clean:
	rm kernel
