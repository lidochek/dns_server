#pragma once
#include "parser.hpp"

namespace resolver {
    std::vector<unsigned char> Resolve(parser::Header header, const std::vector<parser::Question>& questions, bool tcp);
}
