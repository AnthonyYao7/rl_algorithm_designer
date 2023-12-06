//
// Created by anthony on 8/18/23.
//

#ifndef CODEMEALIFE_TESTER_H
#define CODEMEALIFE_TESTER_H

#include <functional>

/**
 *
 * @tparam I input type
 * @tparam O must be default constructible, and default constructed version must be a valid output
 */
template <typename I, typename O>
class Tester {
    typedef std::function<std::pair<I, O>(int)> TestCaseGenerator;
    typedef std::function<void(const I&, O&)> Testee;
    typedef std::function<bool(O&, O&)> Grader;
    typedef std::function<O(I&)> OutputPreparer;
    typedef std::function<void(O&, O&, I&)> Cleanup;

    TestCaseGenerator test_case_gen;
    OutputPreparer output_prepare;
    Testee testee;
    Grader grader;
    Cleanup cleanup;

    unsigned int tests_run = 0;
public:
    struct TestInfo
    {
        bool passed;
    };

    struct TestSuiteInfo
    {
        double passed;
    };

    /**
     * @param test_case_gen takes a random seed to generate test cases with. pass zero to generate a random test case with the last set seed or a positive number to set the seed
     * @param testee a wrapper function which takes references to an input object and an output object and calls the generated function on the input and output
     * @param grader a function which takes the output from the tested function and compares it against ground truth
     */
    Tester(TestCaseGenerator test_case_gen,
           OutputPreparer output_prepare,
           Testee testee,
           Grader grader,
           Cleanup cleanup)
        : test_case_gen(test_case_gen),
        output_prepare(output_prepare),
        testee(testee),
        grader(grader),
        cleanup(cleanup) {}


    TestInfo test_one()
    {
        auto test_case = test_case_gen(tests_run + 1);

        O o = output_prepare(test_case.first);

        testee(test_case.first, o);

        return { grader(test_case.second, o) };
    }

    TestSuiteInfo test(unsigned int num_tests)
    {
        long passed = 0;

        for (unsigned int i = 0; i < num_tests; ++i)
        {
            auto res = test_one();

            if (res.passed)
                ++passed;
        }

        return { (double) passed / num_tests };
    }
};


#endif //CODEMEALIFE_TESTER_H
