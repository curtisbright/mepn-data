all: minimal simple search sieving.start sieving merge organize summary calc check
minimal: minimal.c
	gcc minimal.c -o minimal -Ofast -lgmp -std=c99 -DPRINTSTATS -DPRINTITER -DPRINTDATA
simple: simple.c
	gcc simple.c -o simple -Ofast -lgmp -std=c99
search: search.c
	gcc search.c -o search -Ofast -lgmp -std=c99
sieving.start: sieving.start.c
	gcc sieving.start.c -o sieving.start -Ofast -lgmp -std=c99
sieving: sieving.c
	gcc sieving.c -o sieving -Ofast -lgmp -std=gnu99
merge: merge.c
	gcc merge.c -o merge -Ofast -lgmp -std=c99
organize: organize.c
	gcc organize.c -o organize -Ofast -lgmp -std=c99
summary: summary.c
	gcc summary.c -o summary -Ofast -std=c99
calc: calc.c
	gcc calc.c -o calc -Ofast -lgmp -std=c99
check: check.c
	gcc check.c -o check -Ofast -lgmp -std=c99
clean:
	rm -f minimal simple search sieving.start merge organize summary
