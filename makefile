kernel: kernel.c
	gcc kernel.c -o kernel -O3 -lgmp -std=c99 -DPRINTSTATS -DPRINTITER -DPRINTDIVISORSQUARE -DPRINTDIVISORCUBE -DPRINTDATA $(FLAGS)
debug: FLAGS = -g -pg
debug: kernel
clean:
	rm kernel
checker: checker.c pprime_p.c
	gcc checker.c -o checker -O3 -lgmp -lm -std=c99 -I ~/gmp-5.1.0
