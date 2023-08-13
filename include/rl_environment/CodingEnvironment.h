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

#include "Symbol.h"
#include "Trie.h"


class CodingEnvironment {
public:
    static const int max_children = 20;

    typedef unsigned long long convert_t;
    typedef Trie<Symbol<convert_t>, max_children> Rule;
    typedef Symbol<convert_t> Action;
    typedef std::pair<const std::array<Action, max_children>*, int> ActionList;

private:
    std::unordered_map<std::string, convert_t> symbol_conversion;
    std::vector<std::string> number_conversion;

    std::vector<Rule> rules;
    std::vector<bool> is_terminal;

    std::vector<std::string> resolve_strings;

    std::stack<Rule::Iterator> rule_stack;
    std::stringstream code; // the exciting part

public:
    explicit CodingEnvironment(const std::string& grammar_file,  const std::string& lex_file);
    ~CodingEnvironment();

    void parse_grammar_file(const std::string& grammar_file);
    void parse_lex_file(const std::string& lex_file);

    convert_t convert(const std::string& symbol);
    std::string revert(convert_t index) const;

    Rule& get_rule(convert_t index);
    bool get_terminal(convert_t index) const;

    ActionList getActionSpace();
    void applyAction(const Action& action);

    std::string resolve_action(const Action& action);

    std::string get_code() const;
    void reset();

    // std::vector<Observation> getObservations();

    // Reward getReward();

};


#endif //RL_ALGORITHM_DEVELOPER_CODINGENVIRONMENT_H
