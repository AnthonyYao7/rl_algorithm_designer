//
// Created by anthony on 7/18/23.
//

#include <fstream>
#include <iostream>
#include <utility>
#include <utility>
#include <vector>
#include <string>
#include <regex>
#include <string_view>
#include <algorithm>
#include <sys/wait.h>
#include <numeric>

#include "rl_environment/CodingEnvironment.h"
#include "rl_environment/Symbol.h"
#include "rl_environment/Trie.h"


std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
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

const int CodingEnvironment::max_children;
const int CodingEnvironment::max_identifiers;


CodingEnvironment::CodingEnvironment(
        const std::string& grammar_file,
        const std::string& lex_file,
        int max_actions,
        const std::string& function_name,
        const std::string& return_type,
        const std::vector<std::string>& param_types,
        std::function<double(int, int, const char*)> reward_function) :
        max_actions(max_actions),
        reward_function(std::move(reward_function)){
    /* Push on the zeroth element because we cannot use zero as a token */
    number_conversion.emplace_back();
    rules.emplace_back();
    is_terminal.emplace_back();
    resolve_strings.emplace_back();

    param_count = param_types.size();

    /* Parse grammar file */
    parse_grammar_file(grammar_file);
    /* Parse lex file */
    parse_lex_file(lex_file);
//    init_stack();

    preprocess_function_signature(function_name, return_type, param_types);
    code.reserve(function_signature.length() + CODE_RESERVE_FACTOR * max_actions);
//    code.append(function_signature);

    preprocess_function_signature_tokenized(function_name, return_type, param_types);
    code_tokenized.reserve(max_actions);

    reset(); // (re)set
}


void CodingEnvironment::parse_grammar_file(const std::string &grammar_file)
{
    std::ifstream f(grammar_file);

    if (not f.is_open()){
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

            if (line[1] == 't'){ /* line = "%token ...", line[1] == 't' */
                auto tokens = split(line.substr(7), " ");

                for (const std::string& token : tokens)
                    convert(token);
            }
            else if (line[1] == '%'){ /* line = "%%", line[1] == '%' */
                in_main_section = true;
            }
        } else {// in_main_section
            if (line.length() == 0)
                continue;

            if (line[0] == '%')
                break;

            if (line[0] == '\t'){  // sequence
                if (line[1] == ':' or line[1] == '|'){
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
            else{  // Rule symbol name
                current_symbol = convert(line);
                current_rule = &get_rule(current_symbol);
            }
        }
    }
}


void CodingEnvironment::parse_lex_file(const std::string &lex_file){
    std::ifstream lex(lex_file);

    if (not lex.is_open()){
        std::cout << "Lex file is not open" << std::endl;
        exit(1);
    }

    std::string line;

    const std::regex token_definition_pattern(R"lit("([^"\\)]+[^"\\)]+)".*\(([A-Za-z_]+)\))lit");
    std::smatch match;

    while (getline(lex, line)){
        if (std::regex_search(line, match, token_definition_pattern)){
            std::string resolve_to = match[1];
            std::string symbol = match[2];
            convert_t ind = convert(symbol);
            set_resolve_action(ind, resolve_to);
        }
    }
}


CodingEnvironment::convert_t CodingEnvironment::convert(const std::string& symbol){
    std::string sym = symbol;
    if (sym[0] == '\'')
        sym = sym[1];

    std::unordered_map<std::string, convert_t>::iterator it;
    if ((it = symbol_conversion.find(sym)) == symbol_conversion.end()){
        convert_t index = symbol_conversion.size() + 1;

        symbol_conversion.emplace(sym, index);
        number_conversion.push_back(sym);

        if (sym == "IDENTIFIER")// or sym == "CONSTANT")
            is_terminal.push_back(false);
        else
            is_terminal.push_back((sym[0] >= 'A' and sym[0] <= 'Z') or sym.length() == 1);

        if (sym.length() == 1)
            set_resolve_action(index, sym);

        return index;
    }

    return it->second;
}


std::string CodingEnvironment::revert(convert_t index) const
{
    if (index >= (int)this->number_conversion.size())
        return identifier_to_string(index - symbol_conversion.size() - 1);

    return number_conversion[index];
}


CodingEnvironment::Rule& CodingEnvironment::get_rule(convert_t index)
{
    while ((int)rules.size() <= index)
        rules.emplace_back();

    return rules[index];
}


bool CodingEnvironment::get_terminal(convert_t index) const {
    if (index < (int)is_terminal.size())
        return is_terminal[index];

    throw std::runtime_error("Index not found in get_terminal");
}


CodingEnvironment::ActionList CodingEnvironment::get_action_space(){
    if (in_terminal_state()) {
        return {nullptr, 0};
    }

    if (rule_stack.top() == get_rule(convert("IDENTIFIER")).begin()) /* Very jank way of telling if the most recent action is identifier */
    {
        for (int i = 0; i < identifier_count; ++i)
            identifier_list[i] = Action(symbol_conversion.size() + i + 1, true);

        if (identifier_count != max_identifiers)
            identifier_list[identifier_count] = Action(symbol_conversion.size() + identifier_count + 1, true);

        return {&identifier_list, std::min(identifier_count + 1, CodingEnvironment::max_children)};
    }

    if (rule_stack.top() == get_rule(convert("CONSTANT")).begin())
    {
        return {nullptr, 0};
    }

    ActionList opts = rule_stack.top().get_options();

    while (opts.second == 1) {
        apply_action((*(opts.first))[0]);

        if (rule_stack.empty()) break;

        opts = rule_stack.top().get_options();
    }

    return opts;
}

/**
 * @param action Must be a member of current action space
 */
void CodingEnvironment::apply_action(const Action& action)
{
    if (action.s == 0){ /* last symbol in rule */
        if (!rule_stack.empty())
            this->rule_stack.pop();
        return;
    }

    ++num_actions;

    if (action.s > (int)symbol_conversion.size()) /* since indices start at 1 */
    {
        int identifier_num = action.s - symbol_conversion.size();
        if (identifier_num == identifier_count + 1)
            ++identifier_count;

        code += ' ';
        code += identifier_to_string(identifier_num);

        code_tokenized.emplace_back(identifier_num);

        rule_stack.pop(); /* Pops off the identifier "rule" */
        return;
    }

    rule_stack.top().inc(action); /* Apply action to the most recent rule, the rule it is a part of */

    if (action.terminal){
        code += ' ';
        code += resolve_action(action); /* Add actual string representation into code */

        code_tokenized.emplace_back(action.s);
    }else{
        rule_stack.emplace(get_rule(action.s).begin()); /* start of a new rule... */
    }
}


void CodingEnvironment::set_resolve_action(CodingEnvironment::convert_t index, const std::string &resolve_to)
{
    while ((int)resolve_strings.size() <= index)
        resolve_strings.emplace_back();

    resolve_strings[index] = resolve_to;
}


std::string CodingEnvironment::resolve_action(const Action& action)
{
    // besides a couple of special cases (identifier, constants, string literal, sizeof [get rid of sizeof]),
    // resolving a symbol is pretty much just checking if it is a terminal symbol and if it is,
    // returning the string associated with it

    if (not action.terminal)
        return "";

    if (action.s == 0)
        return "";

    if (action.s < (int)resolve_strings.size() and (int)resolve_strings[action.s].length() > 0)
        return resolve_strings[action.s];

    const std::string symbol_string = revert(action.s);

    if (symbol_string == "CONSTANT"){
        return "1";
    }

    return "";
}


std::string CodingEnvironment::identifier_to_string(int id){
    if (id > 50){
        throw std::runtime_error("Identifier to large");
    }

    if (id == 0)
        return "a";

    std::string ret;
    while (id > 0){
        ret += (id % 26) + 'a';
        id /= 26;
    }

    std::reverse(ret.begin(), ret.end());
    return ret;
}


const std::string & CodingEnvironment::get_code() const{
    return code;
}


void CodingEnvironment::reset()
{
    rule_stack = std::stack<Rule::Iterator>();
    init_stack();

    code.clear();
    code += function_signature;

    code_tokenized.clear();
    code_tokenized.insert(
            std::end(code_tokenized),
            std::begin(function_signature_tokenized),
            std::end(function_signature_tokenized));

    num_actions = 0;
    identifier_count = param_count;
}


void CodingEnvironment::init_stack() {
    const std::vector<std::string> init_state = {"translation_unit", "external_declaration", "function_definition"};
    const int inc[4] = {1, 1, 3};

    for (size_t i = 0; i < init_state.size(); ++i) {
        const std::string& s = init_state[i];

        rule_stack.push(get_rule(convert(s)).begin());

        for (int j = 0; j < inc[i]; ++j) {
            rule_stack.top().inc((*rule_stack.top().get_options().first)[0]);
        }
    }
}


bool CodingEnvironment::in_terminal_state() const {
    return this->rule_stack.empty() or num_actions > max_actions;
}


CodingEnvironment::Reward CodingEnvironment::get_reward() const {
    int prog_fd[2], gcc_err_fd[2];

    if (pipe(prog_fd) == -1 or pipe(gcc_err_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // print out the program
    if (fork() == 0) {
        close(STDOUT_FILENO);
        dup(prog_fd[1]);
        close(prog_fd[0]);
        close(prog_fd[1]);

        printf("%s", code.c_str());
        exit(0);
    }

    pid_t gcc_pid;

    // compile the program
    if ((gcc_pid=fork()) == 0) {
        close(STDIN_FILENO);
        dup(prog_fd[0]);
        close(prog_fd[1]);
        close(prog_fd[0]);

        close(STDERR_FILENO);
        dup2(gcc_err_fd[1], STDERR_FILENO);
        close(gcc_err_fd[0]);
        close(gcc_err_fd[1]);

        pid_t cur_pid = getpid();
        std::string path = BIN_GEN_PATH + std::to_string((int) cur_pid) + ".out";
        const char* args[] = {"gcc", "-fsanitize=address", "-static-libasan", "-g", "-o", path.c_str(), "-x", "c", "-pipe", "-"};
        execlp(args[0], args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], NULL);

        perror("gcc execlp failed");
        exit(1);
    }

    close(prog_fd[0]);
    close(prog_fd[1]);
    close(gcc_err_fd[1]);

    char buf[1024];
    ssize_t bytesRead;
    while ((bytesRead=read(gcc_err_fd[0], buf, sizeof(buf) - 1)) > 0) {
        // TODO: Write handler for error output
    }

    close(gcc_err_fd[0]);

    int status;
    /* wait for gcc to finish running */
    if (waitpid(gcc_pid, &status, 0) < 0) {
        std::cout << "Wait failed\n";
        exit(EXIT_FAILURE);
    }

    int gcc_ec=-1;
    // TODO: Implement compile error checking
    if (WIFEXITED(status)) {
        switch (gcc_ec=WEXITSTATUS(status)) {
            case 0:
                std::cout << code << std::endl;
                break;
            case 1:
                std::cout << "Did not compile\n";
                break;
            default:
                std::cout << "Do not know what happened\n";
                break;
        }
    } else {
        // TODO: Handle what happens if the program does something besides exiting (loop the wait?)
    }

    /* wait for program printing process */
    wait(nullptr);

    if (gcc_ec != 0) {
        return reward_function(gcc_ec, 0, nullptr);
    }

    /* run the generated program */
    std::string path = BIN_GEN_PATH + std::to_string((int) gcc_pid) + ".out";
    pid_t gen_prog_pid;
    if ((gen_prog_pid=fork()) == 0) {
        execlp(path.c_str(), path.c_str(), NULL);
        exit(1);
    }

    /* wait for program to finish */
    if (waitpid(gen_prog_pid, &status, 0) < 0) {
        std::cout << "Wait failed\n";
        exit(EXIT_FAILURE);
    }

    int prog_ec=-1;
    // TODO: Same as above
    if (WIFEXITED(status)) {
        switch (prog_ec=WEXITSTATUS(status)) {
            case 0:
                std::cout << "Wow the generated program actually ran\n";
                break;
            case 1:
                std::cout << "Generated program failed with exit code 1\n";
                break;
            default:
                std::cout << "Not quite sure what the generated program did\n";
                break;
        }
    } else {
        // TODO: Same as above
    }

    return reward_function(gcc_ec, prog_ec, nullptr);
}

void CodingEnvironment::preprocess_function_signature(const std::string &function_name, const std::string &return_type,
                                                      const std::vector<std::string> &param_types) {
    function_signature.reserve(
            function_name.length() +
            return_type.length() +
            std::accumulate(
                    param_types.begin(),
                    param_types.end(),
                    0,
                    [] (int a, const std::string& b) { return a + b.length(); }) +
            4 * param_types.size());

    function_signature += parse_type(return_type);
    function_signature += ' ';
    function_signature += function_name;
    function_signature += '(';

    // simply unrolling first iteration in order to prevent branching for the comma
    if (!param_types.empty()) {

        function_signature += parse_type(param_types[0]);
        function_signature += ' ';
        function_signature += identifier_to_string((int)0);
    }

    for (unsigned int i = 1; i < param_types.size(); ++i) {
        function_signature += ',';
        function_signature += parse_type(param_types[i]);
        function_signature += ' ';
        function_signature += identifier_to_string((int)i);
    }

    function_signature += ')';
    function_signature.shrink_to_fit();
}

void CodingEnvironment::preprocess_function_signature_tokenized(const std::string &function_name,
                                                                const std::string &return_type,
                                                                const std::vector<std::string> &param_types) {
    function_signature_tokenized.reserve(3 + 2 * param_types.size());

    function_signature_tokenized.emplace_back(convert(return_type));
    function_signature_tokenized.emplace_back(FUNCTION_TOKEN_VALUE);
    function_signature_tokenized.emplace_back(convert("("));

    if (!param_types.empty()) {
        parse_type_token(param_types[0]);
        function_signature_tokenized.emplace_back(symbol_conversion.size() + 1);
    }

    for (int i = 1; i < param_types.size(); ++i) {
        function_signature_tokenized.emplace_back(convert(","));
        parse_type_token(param_types[i]);
    }

    function_signature_tokenized.emplace_back(convert(")"));

    function_signature_tokenized.shrink_to_fit();
}

const std::vector<CodingEnvironment::Token> & CodingEnvironment::get_observations() {
    return code_tokenized;
}

std::string CodingEnvironment::parse_type(const std::string &type_name) {
    return resolve_action({
        convert(
                type_name.back() == ']' ?
                type_name.substr(0, type_name.size() - 2) :
                type_name),
        true
    }) + (type_name.back() == ']' ? "[]" : "");
}

void CodingEnvironment::parse_type_token(const std::string &type_name) {
    if (type_name.back() == ']') {
        function_signature_tokenized.emplace_back(
            convert(
            type_name.substr(0, type_name.size() - 2)));
        function_signature_tokenized.emplace_back(convert("["));
        function_signature_tokenized.emplace_back(convert("]"));
    } else {
        function_signature_tokenized.emplace_back(convert(type_name));
    }
}
