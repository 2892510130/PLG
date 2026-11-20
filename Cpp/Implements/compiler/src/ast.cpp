#include "ast.hpp"

namespace ast {

TypePtr Type::named(std::string n) {
    auto t = std::make_unique<Type>();
    t->kind = Kind::Named;
    t->name = std::move(n);
    return t;
}
TypePtr Type::unit() {
    auto t = std::make_unique<Type>();
    t->kind = Kind::Unit;
    return t;
}
TypePtr Type::fn(std::vector<TypePtr> params, TypePtr ret) {
    auto t = std::make_unique<Type>();
    t->kind = Kind::Fn;
    t->params = std::move(params);
    t->ret = std::move(ret);
    return t;
}

std::string Type::toString() const {
    switch (kind) {
        case Kind::Named: return name;
        case Kind::Unit:  return "()";
        case Kind::Fn: {
            std::string s = "fn(";
            for (size_t i = 0; i < params.size(); ++i) {
                if (i) s += ", ";
                s += params[i]->toString();
            }
            s += ")";
            if (ret) s += " -> " + ret->toString();
            return s;
        }
    }
    return "?";
}

}  // namespace ast