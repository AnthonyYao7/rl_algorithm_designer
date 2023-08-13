//
// Created by anthony on 7/25/23.
//

#include <iostream>
#include <random>

#include "rl_environment/CodingEnvironment.h"


CodingEnvironment::Action choose_random_action(CodingEnvironment& env)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 20);

    auto [action_list, num_actions] = env.getActionSpace();
    int choice = dis(gen) % num_actions;
    return (*action_list)[choice];
}



void randomly_walk(CodingEnvironment& env, int num_iterations)
{
    for (int i = 0; i < num_iterations; ++i)
    {
        CodingEnvironment::Action action = choose_random_action(env);
        env.applyAction(action);
    }
}



int main()
{
    CodingEnvironment env("../../viz/ansi.c.grammar.y", "../../viz/ansi.c.grammar.l");

    bool running = true;
    while (running)
    {
        auto actions = env.getActionSpace();
        auto& list = *actions.first;

        for (int i = 0; i < actions.second; ++i)
        {
            std::cout << i + 1 << ". " << env.revert(list[i].s) << std::endl;
        }

        std::string choice;
        std::cin >> choice;

        if (choice == "done")
        {
            std::cout << env.get_code() << std::endl;

            running = false;
            continue;
        }

        int c = std::stoi(choice);

        env.applyAction(list[c - 1]);
        std::cout << std::endl;
    }
}
