//
// Created by anthony on 12/9/23.
//

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef long input[];
typedef long output;

output func(input in);
output gt(input in);

bool create_input_and_check(unsigned int seed) {
    srand(seed);

    long a[100];
    memset(a, 0, sizeof(long) * 100);

    long res = func(a), truth = gt(a);

    return res == truth;
}

void print_representative_and_output(unsigned int seed) {
    srand(seed);

    long a[100];
    for (int i = 0; i < 100; ++i) {
        a[i] = i;
        printf("%ld", a[i]);
    }

    long res = func(a);

    printf("%ld", res);
//    printf("\n%c%c%c%c\n", *((char*)(&res)),
//           *(((char*)(&res))+1), *(((char*)(&res))+2), *(((char*)(&res)))+3);
}

output gt(input in) {
    return 1;
}
