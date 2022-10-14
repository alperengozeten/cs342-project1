#include <stdio.h>

int main(int argc, char* argv[]) {

    if ( argc < 5 ) {
        printf("You have entered insufficient number of arguments! Usage: pword <msgsize> <outfile> <N> <infile1> .... <infileN>\n");
        return -1;
    }
    return 0;
}