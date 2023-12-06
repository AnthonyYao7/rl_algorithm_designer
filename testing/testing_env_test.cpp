//
// Created by anthony on 8/22/23.
//

#include <random>
#include <vector>
#include <iostream>

#include "testing_harness/Tester.h"


struct ZOKnapsackInput
{
    int* w;
    int* v;
    int n;
    int c;
};

typedef int ZOKnapsackOutput;
typedef Tester<ZOKnapsackInput, ZOKnapsackOutput> ZOKnapsackTester;


int solve_zoknapack(const int* w, const int* v, int n, int c)
{
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(c + 1));

    for (int i = 0; i <= n; ++i)
    {
        for (int j = 0; j <= c; ++j)
        {
            if (i == 0 || j == 0)
            {
                dp[i][j] = 0;
            }
            else if (w[i - 1] <= j)
            {
                dp[i][j] = std::max(v[i - 1] + dp[i - 1][j - w[i - 1]], dp[i - 1][j]);
            }
            else
            {
                dp[i][j] = dp[i - 1][j];
            }
        }
    }

    return dp[n][c];
}


std::pair<ZOKnapsackInput, ZOKnapsackOutput> test_case_generator(int a)
{
    static std::mt19937 gen(a);
    static std::uniform_int_distribution<> c_generator(1, 100);
    static std::uniform_int_distribution<> n_generator(1, 10);
    static std::uniform_int_distribution<> w_generator(1, 20);
    static std::uniform_int_distribution<> v_generator(1, 10000);

    if (a != 0)
        gen = std::mt19937(a);

    ZOKnapsackInput ret_input;

    ret_input.c = c_generator(gen);
    ret_input.n = n_generator(gen);

    ret_input.w = new int[ret_input.n];
    ret_input.v = new int[ret_input.n];

    for (int i = 0; i < ret_input.n; ++i)
    {
        ret_input.w[i] = w_generator(gen);
        ret_input.v[i] = v_generator(gen);
    }

    ZOKnapsackOutput ret_output = solve_zoknapack(ret_input.w, ret_input.v, ret_input.n, ret_input.c);

    return {ret_input, ret_output};
}


int output_prepare(ZOKnapsackInput& in)
{
    return 0;
}


void cleanup(ZOKnapsackOutput& tco, ZOKnapsackOutput& sol, ZOKnapsackInput& tci)
{
    delete[] tci.w;
    delete[] tci.v;
}


// same function, just different format
void solution(const int* w, const int* v, int n, int c, int* r)
{
    *r = solve_zoknapack(w, v, n, c);
}


void testee(const ZOKnapsackInput& in, ZOKnapsackOutput& out)
{
    solution(in.w, in.v, in.n, in.c, &out);
}


bool eq(int& a, int& b)
{
    return a == b;
}


int main()
{
    ZOKnapsackTester t(test_case_generator, output_prepare, testee, eq, cleanup);

    auto res = t.test(100);
    std::cout << res.passed << std::endl;


}
