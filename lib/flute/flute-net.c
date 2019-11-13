#include <stdio.h>
#include <stdlib.h>
#include "flute.h"

int main()
{
    int d=0;
    int x[MAXD], y[MAXD];
    Tree flutetree;
    int flutewl;
    

    //readLUT();
    //printf("=======\n");
    readStringLUT();
    //return 0;
    while (!feof(stdin)) {
        scanf("%d %d\n", &x[d], &y[d]);
        d++;
    }
    flutetree = flute(d, x, y, ACCURACY);\
    printf("FLUTE wirelength = %d, degree %d\n", flutetree.length, flutetree.deg );
    
    if( flutetree.branch )
        printf("branch size= %d\n", sizeof(flutetree.branch) );
        for(int i=0; i<d; i++){
            printf("%3d: (%5d %5d)\n", i, x[i], y[i] );
        }
        printf("--- steiner points ---\n");
        for(int i=0; i<2*d-2; i++){
            printf("%3d: (%5d %5d) %3d\n", i
                , flutetree.branch[i].x
                , flutetree.branch[i].y
                , flutetree.branch[i].n );
        }
        int gx=x[0] ,gy=y[0] ,mx=x[0], my=y[0];
        for(int i=0; i<2*d-2; i++){
            gx = gx < flutetree.branch[i].x? flutetree.branch[i].x: gx;
            gy = gy < flutetree.branch[i].y? flutetree.branch[i].y: gy;
            mx = mx > flutetree.branch[i].x? flutetree.branch[i].x: mx;
            my = my > flutetree.branch[i].y? flutetree.branch[i].y: my;
            printf("set arrow from %5d,%5d to %5d,%5d\n"
                , flutetree.branch[i].x
                , flutetree.branch[i].y
                , flutetree.branch[flutetree.branch[i].n].x
                , flutetree.branch[flutetree.branch[i].n].y );
        }
        printf("set xrange [%6d:%6d]\n", mx-100, gx + 100 );
        printf("set yrange [%6d:%6d]\n", my-100, gy + 100 );
        printf("set output \"steiner.png\"\n");
        printf("set term png size 500,500 truecolor\n");
        printf("plot 0\n");

    if( flutetree.branch )\
	   free( flutetree.branch );

    flutewl = flute_wl(d, x, y, ACCURACY);\
    printf("FLUTE wirelength (without RSMT construction) = %d\n", flutewl);
    endLUT();
}
