//
// Created by anthony on 12/7/23.
//

/*
 * The tester has to not only run test cases, but also give the output of the agent
 * on a good representative for the problem
 * */

#include <stdio.h>
#include <stdbool.h>

#define TEST_ROUNDS 100

bool create_input_and_check(unsigned int seed);
void print_representative_and_output(unsigned int seed);
double run_tests();

int main() {
    printf("%f\n", run_tests());
    print_representative_and_output(TEST_ROUNDS);
}

double run_tests() {
    long count = 0;
    for (int i = 0; i < TEST_ROUNDS; ++i) {
        count += (int) (create_input_and_check(i+1));
    }
    return (double)count/TEST_ROUNDS;
}
