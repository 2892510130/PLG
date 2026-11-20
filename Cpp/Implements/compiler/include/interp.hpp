#pragma once

#include "ast.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct RuntimeError {
    int line, column;
    std::string message;
};

class Interpreter {
public:
    // monostate == unit
    using Value = std::variant<std::monostate, int64_t, double, bool, std::string>;

    explicit Interpreter(std::ostream& out = std::cout) : out_(out) {}

    // Returns process exit code. Throws RuntimeError on failure.
    int run(ast::Program& program);

    static std::string valueToString(const Value& v);

private:
    struct ReturnSignal { Value value; };

    std::ostream& out_;
    std::unordered_map<std::string, const ast::FnDecl*> functions_;
    std::vector<std::unordered_map<std::string, Value>> scopes_;
    int call_depth_ = 0;
    static constexpr int kMaxDepth = 10000;

    void pushScope() { scopes_.emplace_back(); }
    void popScope()  { scopes_.pop_back(); }
    Value& lookupVar(const std::string& name, int line, int col);

    Value evalExpr(ast::Expr& e);
    void  execStmt(ast::Stmt& s);
    Value callFunction(const ast::FnDecl* fn, std::vector<Value> args, int line, int col);
    Value callPrint(const std::vector<Value>& args);
};