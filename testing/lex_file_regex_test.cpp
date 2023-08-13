//
// Created by anthony on 8/9/23.
//

#include <regex>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

int main()
{
    std::string lex_file = "../../viz/ansi.c.grammar.l";

    std::ifstream lex(lex_file);
    if (not lex.is_open())
    {
        std::cout << "not open" << std::endl;
        exit(0);
    }

    std::regex token_definition_pattern(R"lit("([^"\\)]+[^"\\)]+)".*\(([A-Za-z_]+)\))lit");
    std::smatch match;
    std::string line;
    while (getline(lex, line))
    {
        if (std::regex_search(line, match, token_definition_pattern))
        {
            std::cout << line << std::endl;
            std::cout << match[1] << " --- " << match[2] << std::endl << std::endl;
        }
    }
}
