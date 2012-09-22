compile:
	gcc kernel.c -o kernel -O3 -lgmp -std=c99 -DPRINTSTATS -DPRINTITER
debug:
	gcc kernel.c -o kernel -O3 -lgmp -std=c99 -DPRINTSTATS -g -pg
clean:
	rm kernel
