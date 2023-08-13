//
// Created by anthony on 7/18/23.
//

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <string_view>

#include "rl_environment/CodingEnvironment.h"
#include "rl_environment/Symbol.h"
#include "rl_environment/Trie.h"


std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}


CodingEnvironment::CodingEnvironment(const std::string& grammar_file, const std::string& lex_file)
{
    /* Push on the zeroth element because we cannot use zero as a token */
    number_conversion.emplace_back();
    rules.emplace_back();
    is_terminal.emplace_back();
    resolve_strings.emplace_back();

    /* Parse grammar file */
    parse_grammar_file(grammar_file);

    rule_stack.push(get_rule(convert("translation_unit")).begin());

    /* Parse lex file */
    parse_lex_file(lex_file);
}


void CodingEnvironment::parse_grammar_file(const std::string &grammar_file)
{
    std::ifstream f(grammar_file);

    if (not f.is_open())
    {
        std::cout << "Grammar file failed to open" << std::endl;
        exit(1);
    }

    std::string line;

    bool in_main_section = false;
    convert_t current_symbol;
    Rule* current_rule = nullptr;

    while (std::getline(f, line))
    {
        if (line[0] == '/') /* Comment */
            continue;

        if (not in_main_section)
        {
            if (line.length() == 0)
                continue;

            if (line[1] == 't') /* line = "%token ...", line[1] == 't' */
            {
                auto tokens = split(line.substr(7), " ");

                for (const std::string& token : tokens)
                    convert(token);
            }
            else if (line[1] == '%') /* line = "%%", line[1] == '%' */
            {
                in_main_section = true;
            }
        }
        else // in_main_section
        {
            if (line.length() == 0)
                continue;

            if (line[0] == '%')
                break;

            if (line[0] == '\t')  // sequence
            {
                if (line[1] == ':' or line[1] == '|')
                {
                    std::string seq = line.substr(3);
                    std::vector<std::string> elements = split(seq, " ");

                    std::vector<Symbol<convert_t>> sym_seq;

                    std::transform(elements.begin(), elements.end(), std::back_inserter(sym_seq), [this] (const std::string& s) {
                        convert_t cv = convert(s);
                        return Symbol<convert_t>(cv, get_terminal(cv));
                    });

                    current_rule->insert_sequence(sym_seq);
                }
            }
            else  // Rule symbol name
            {
                current_symbol = convert(line);
                current_rule = &get_rule(current_symbol);
            }
        }
    }
}


void CodingEnvironment::parse_lex_file(const std::string &lex_file)
{
    std::ifstream lex(lex_file);

    if (not lex.is_open())
    {
        std::cout << "Lex file is not open" << std::endl;
        exit(1);
    }

    std::string line;

    std::regex token_definition_pattern(R"lit("([^"\\)]+[^"\\)]+)".*\(([A-Za-z_]+)\))lit");
    std::smatch match;

    while (getline(lex, line))
    {
        if (std::regex_search(line, match, token_definition_pattern))
        {
            std::string resolve_to = match[1];
            std::string symbol = match[2];

            convert_t ind = this->convert(symbol);

            while (this->resolve_strings.size() <= ind)
                this->resolve_strings.emplace_back();

            this->resolve_strings[ind] = resolve_to;
        }
    }
}


CodingEnvironment::convert_t CodingEnvironment::convert(const std::string& symbol)
{
    std::string sym = symbol;
    if (sym[0] == '\'')
        sym = sym[1];

    std::unordered_map<std::string, convert_t>::iterator it;
    if ((it = symbol_conversion.find(sym)) == symbol_conversion.end())
    {
        convert_t index = symbol_conversion.size() + 1;

        symbol_conversion.emplace(sym, index);
        number_conversion.push_back(sym);
        is_terminal.push_back((sym[0] >= 'A' and sym[0] <= 'Z') or sym.length() == 1);

        return index;
    }

    return it->second;
}


std::string CodingEnvironment::revert(convert_t index) const
{
    if (index >= this->number_conversion.size())
        throw std::runtime_error("Index not found in revert");

    return number_conversion[index];
}


CodingEnvironment::Rule& CodingEnvironment::get_rule(convert_t index)
{
    while (rules.size() <= index)
        rules.emplace_back();

    return rules[index];
}


bool CodingEnvironment::get_terminal(convert_t index) const
{
    if (index < is_terminal.size())
        return is_terminal[index];

    throw std::runtime_error("Index not found in get_terminal");
}


CodingEnvironment::ActionList CodingEnvironment::getActionSpace()
{
    if (this->rule_stack.empty())
    {
        std::cout << "Rule stack empty, done?" << std::endl;
        return {nullptr, 0};
    }

    return this->rule_stack.top().get_options();
}

/**
 *
 * @param action Must be a member of current action space
 */
void CodingEnvironment::applyAction(const Action& action)
{
    this->rule_stack.top().inc(action); /* Apply action to the most recent rule, the rule it is a part of */

    if (action.terminal)
    {
        this->code << ' ' << resolve_action(action);

        while (this->rule_stack.top().is_end()) /* The top of the stack is a leaf node in its trie, i.e., it is the last symbol in the rule */
            this->rule_stack.pop();
    }
    else
    {
        this->rule_stack.emplace(get_rule(action.s).begin()); /* start of a new rule... */
    }
}


std::string CodingEnvironment::resolve_action(const Action& action)
{
    // besides a couple of special cases (identifier, constants, string literal, sizeof [get rid of sizeof]),
    // resolving a symbol is pretty much just checking if it is a terminal symbol and if it is,
    // returning the string associated with it

    if (not action.terminal)
        return "";

    if (action.s < resolve_strings.size() and resolve_strings[action.s].length() > 0)
        return resolve_strings[action.s];

    const std::string symbol_string = revert(action.s);

    if (symbol_string == "CONSTANT")
    {
        /**
         * Not implemented
         */

        return "1";
    }

    if (symbol_string == "IDENTIFIER")
    {
        /**
         * Not implemented
         */

        return "a";
    }

    if (symbol_string == "STRING_LITERAL")
    {
        /**
         * Not implemented, might never implement this
         */

        return "";
    }

    return "";
}


std::string CodingEnvironment::get_code() const
{
    return this->code.str();
}


void CodingEnvironment::reset()
{
    rule_stack = std::stack<Rule::Iterator>();
    code.clear();
    code.str("");
}
