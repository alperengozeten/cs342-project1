all: pword tword

pword: pword.c
	gcc -Wall -o pword pword.c -lrt

tword: tword.c
	gcc -Wall -o tword tword.c -lrt

clean:
	rm -fr *~ pword tword