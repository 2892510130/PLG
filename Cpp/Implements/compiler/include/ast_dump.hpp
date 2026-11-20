#pragma once

#include "ast.hpp"

#include <ostream>

namespace ast {
void dump(const Program& p, std::ostream& os);
}