//
// Created by anthony on 7/25/23.
//

#include <iostream>
#include <random>
#include <filesystem>
#include <fstream>
#include <string>

#include "rl_environment/CodingEnvironment.h"

#define NUM_WALKS 10
#define NUM_ITERS 1000000

namespace fs = std::filesystem;

inline const std::string walk_results_path = "results";

/*
 * How to support creating new identifiers?
 * 1. Make IDENTIFIER a non-terminal symbol
 * 2. When the top rule is IDENTIFIER, have get_action_space return a list of all existing variables
 *    and a new variable which does not exist yet (maybe cap the number of times a new variable can be created)
 * 3. Extend the symbol class or the action class in some way to allow for actions besides the symbols in the grammar
 *   - Maybe have some special values of s that are greater than the values of s representing other symbols which mean to use an identifier,
 *   - and the number encodes which variable to resolve to
 * 4. Extend the action apply function to allow for these new symbols or actions
 * */


bool path_exists(const std::string& path){
    return fs::exists(path) and fs::is_directory(path);
}


double reward_function(int gcc_ec, int prog_ec, const char* gcc_logs) {
    return (1-std::abs(gcc_ec)) * (1-std::abs(prog_ec));
}


CodingEnvironment::Action choose_random_action(CodingEnvironment& env)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 20);

    auto [action_list, num_actions] = env.get_action_space();

    if (num_actions == 0)
        return {};

    int choice = dis(gen) % num_actions;
    return (*action_list)[choice];
}


void randomly_walk(CodingEnvironment& env, int num_iterations, int walk_num)
{
    for (int i = 0; i < num_iterations; ++i)
    {
        if (env.in_terminal_state())
        {
            std::cout << "reached terminal state" << std::endl;
            break;
        }

        CodingEnvironment::Action action = choose_random_action(env);

        env.apply_action(action);
    }

    if (not path_exists(walk_results_path))
        fs::create_directory(walk_results_path);

    std::ofstream w(walk_results_path + "/walk_" + std::to_string(walk_num) + ".c");
    const std::string code = env.get_code();
    w.write(code.c_str(), code.length());
    w.close();
}



int main() {
    CodingEnvironment env(
            "../../viz/c_grammar_subset.y",
            "../../viz/ansi.c.grammar.l",
            10000,
            "main",
            "int",
            {"int"},
            reward_function);

//    bool running = true;
//    while (!env.in_terminal_state()) {
//        auto actions = env.get_action_space();
//        auto& list = *actions.first;
//
//        for (int i = 0; i < actions.second; ++i) {
//            std::cout << i + 1 << ". " << env.revert(list[i].s) << std::endl;
//        }
//
//        std::string choice;
//        std::cin >> choice;
//
//        if (choice == "done") {
//            std::cout << env.get_code() << std::endl;
//
//            running = false;
//            continue;
//        }
//
//        int c = std::stoi(choice);
//
//        env.apply_action(list[c - 1]);
//        std::cout << std::endl;
//    }
//
//    std::cout << env.get_code() << '\n';
//
//    env.get_reward();

    for (int i = 0; i < NUM_WALKS; ++i)
    {
        randomly_walk(env, NUM_ITERS, i);
        std::cout << env.get_reward() << '\n';
        env.reset();
    }
}
