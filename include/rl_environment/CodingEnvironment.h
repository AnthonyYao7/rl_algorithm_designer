//
// Created by anthony on 7/18/23.
//

#ifndef RL_ALGORITHM_DEVELOPER_CODINGENVIRONMENT_H
#define RL_ALGORITHM_DEVELOPER_CODINGENVIRONMENT_H

#include <stack>
#include <string>
#include <sstream>
#include <functional>
#include <unordered_map>

#include <cstdlib>

#include "Symbol.h"
#include "Trie.h"

#define BIN_GEN_PATH "../../binaries/"
#define CODE_RESERVE_FACTOR 1

class CodingEnvironment {
public:
    static const int max_children = 50; /* Changed from 20 to accommodate for many different identifiers */
    static const int max_identifiers = 50;

    typedef int convert_t;
    typedef Trie<Symbol<convert_t>, max_children> Rule;
    typedef Symbol<convert_t> Action;
    typedef std::pair<const std::array<Action, max_children>*, int> ActionList;
    typedef double Reward;

    std::string function_signature;

private:
    std::unordered_map<std::string, convert_t> symbol_conversion;
    std::vector<std::string> number_conversion;

    std::vector<Rule> rules;
    std::vector<bool> is_terminal;

    std::vector<std::string> resolve_strings;

    int param_count{0};
    int identifier_count{0};
    std::array<Action, max_children> identifier_list{};

    int max_actions;
    int num_actions{};

    std::stack<Rule::Iterator> rule_stack;
//    std::stringstream code; // the exciting part
    std::string code;

    std::function<double(int, int, const char*)> reward_function;

public:
    explicit CodingEnvironment(
            const std::string& grammar_file,
            const std::string& lex_file,
            int max_actions,
            const std::string& function_name,
            const std::string& return_type,
            const std::vector<std::string>& param_types,
            std::function<double(int, int, const char*)> reward_function);

    void parse_grammar_file(const std::string& grammar_file);
    void parse_lex_file(const std::string& lex_file);

    convert_t convert(const std::string& symbol);
    std::string revert(convert_t index) const;

    Rule& get_rule(convert_t index);
    bool get_terminal(convert_t index) const;

    ActionList get_action_space();
    void apply_action(const Action& action);

    void set_resolve_action(convert_t index, const std::string& resolve_to);
    std::string resolve_action(const Action& action);
    static std::string identifier_to_string(int id);

    const std::string & get_code() const;
    void reset();
    void init_stack();

    bool in_terminal_state() const;

    /*
     * This potentially could be the program, written in postfix order with some a one hot representation for the tokens
     * pop it into a transformer and ur done
     * */
    // std::vector<Observation> getObservations();

    /*
     * ideas:
     * - at each state, check if the current code compiles and keep track of the time since the last time all the code compiled
     *   the longer since the last time the code compiled, the lower the reward
     * - obviously, the agent should be rewarded for code in the following hierarchy starting from the best:
     *   1. Compiles, correct solution with super optimal running time
     *   2. Compiles, correct solution with optimal running time
     *   3. Compiles, correct solution with suboptimal running time
     *   4. Compiles, incorrect solution
     *   5. Compiles, runtime error (address sanitizer, etc)
     *   6. Does not compile
     *   7.
     * */

     Reward get_reward() const;

};


#endif //RL_ALGORITHM_DEVELOPER_CODINGENVIRONMENT_H
