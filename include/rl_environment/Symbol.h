//
// Created by anthony on 7/25/23.
//

#ifndef CODEMEALIFE_SYMBOL_H
#define CODEMEALIFE_SYMBOL_H

#include <concepts>


template <std::equality_comparable T>
struct Symbol
{
    T s;
    bool terminal;

    bool operator == (const Symbol& rhs) const
    {
        return (s == rhs.s) and (terminal == rhs.terminal);
    }
};


#endif //CODEMEALIFE_SYMBOL_H
