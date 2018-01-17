#include "fnv_hash.h"

#include <stdio.h>
#include <malloc.h>

int main() {

    int i, k, r;
    
    double LO = 0;
    double HI = 1000;
    
    int   bucketsSize = 1000;
    long* buckets     = (long*)calloc(bucketsSize,sizeof(long));
    
    for (i = 0; i < bucketsSize; i++) {
        for (k = 0; k < 100; k++) {
            int random_counter = 0;
            for (r = 0; r < 3; r++) {
                float f = fnv_hash(i, k, &random_counter, LO, HI);
                int index = (int)f;
                if (index < 0 || index >= bucketsSize) { fprintf(stderr, "Bad index\n"); return 1; }
                buckets[index]++;
            }
        }
    }

    int  histogramsSize = 1000;
    int* histograms     = (int*)calloc(histogramsSize,sizeof(int));
    
    for (i = 0; i < bucketsSize; i++) {
        int index = (int)buckets[i];
        if (index < 0 || index >= histogramsSize) { fprintf(stderr, "Bad histograms index\n"); return 1; }
        histograms[index]++;
    }

    for (i = 0; i < histogramsSize; i++) {
        if (histograms[i] == 0) continue;
        printf("%d : %d\n", i, histograms[i]);
    }
        
    return 0;
}
