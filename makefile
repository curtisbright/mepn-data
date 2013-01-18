compile: kernel.c
	gcc kernel.c -o kernel -O3 -lgmp -std=c99 -DPRINTSTATS -DPRINTITER
debug: kernel.c
	gcc kernel.c -o kernel -O3 -lgmp -std=c99 -DPRINTSTATS -g -pg
clean:
	rm kernel
checker: checker.c pprime_p.c
	gcc checker.c -o checker -O3 -lgmp -lm -std=c99 -I ~/Desktop/gmp-5.0.4 -L ~/Desktop/gmp-5.0.4/.libs
