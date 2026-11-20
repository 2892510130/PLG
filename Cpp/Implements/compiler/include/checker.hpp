#pragma once

#include "ast.hpp"

#include <string>
#include <unordered_map>
#include <vector>

struct SemanticError { int line, column; std::string message; };

struct CheckResult {
    std::vector<SemanticError> errors;
    bool ok() const { return errors.empty(); }
};

class Checker {
public:
    CheckResult check(ast::Program& program);

private:
    struct VarInfo { ast::TypePtr type; bool is_mut; };
    struct FnInfo  { const ast::FnDecl* decl; };

    std::vector<std::unordered_map<std::string, VarInfo>> scopes_;
    std::unordered_map<std::string, FnInfo> functions_;
    const ast::FnDecl* current_fn_ = nullptr;
    std::vector<SemanticError> errors_;

    void error(int line, int col, std::string msg);

    void pushScope();
    void popScope();
    VarInfo* lookupVar(const std::string& name);
    bool declareVar(const std::string& name, VarInfo info, int line, int col);

    void collectFunctions(const ast::Program& program);
    void checkFn(const ast::FnDecl& fn);
    void checkStmt(const ast::Stmt& stmt);
    ast::TypePtr checkExpr(ast::Expr& expr, const ast::Type* expected = nullptr);
    bool coerce(ast::Expr& expr, const ast::Type& target, const char* context);
};