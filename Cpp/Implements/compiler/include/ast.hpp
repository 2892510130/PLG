#pragma once

#include "lexer.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace ast {

struct Expr; struct Stmt; struct Type;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using TypePtr = std::unique_ptr<Type>;

// ---------- Types ----------
struct Type {
    enum class Kind { Named, Unit, Fn };
    Kind kind = Kind::Named;
    std::string name;               // Named
    std::vector<TypePtr> params;    // Fn
    TypePtr ret;                    // Fn

    static TypePtr named(std::string n);
    static TypePtr unit();
    static TypePtr fn(std::vector<TypePtr> params, TypePtr ret);
    std::string toString() const;
};

// ---------- Expressions ----------
struct Expr {
    int line = 0, column = 0;

    struct IntLit    { int64_t value; };
    struct FloatLit  { double value; };
    struct StringLit { std::string value; };
    struct BoolLit   { bool value; };
    struct Ident     { std::string name; };
    struct Unary     { TokenKind op; ExprPtr operand; };
    struct Binary    { TokenKind op; ExprPtr lhs; ExprPtr rhs; };
    struct Assign    { ExprPtr target; ExprPtr value; };
    struct Call      { ExprPtr callee; std::vector<ExprPtr> args; };
    struct Index     { ExprPtr base; ExprPtr index; };
    struct Field     { ExprPtr base; std::string name; };

    TypePtr inferred_type;

    using Node = std::variant<IntLit, FloatLit, StringLit, BoolLit, Ident,
                              Unary, Binary, Assign, Call, Index, Field>;
    Node node;

    Expr(int l, int c, Node n) : line(l), column(c), node(std::move(n)) {}
};

// ---------- Statements ----------
struct Stmt {
    int line = 0, column = 0;

    struct Let      { std::string name; bool is_mut = false; TypePtr type; ExprPtr init; };
    struct ExprStmt { ExprPtr expr; };
    struct Block    { std::vector<StmtPtr> stmts; };
    struct If       { ExprPtr cond; StmtPtr then_br; StmtPtr else_br; };
    struct While    { ExprPtr cond; StmtPtr body; };
    struct Return   { ExprPtr value; };   // nullable

    using Node = std::variant<Let, ExprStmt, Block, If, While, Return>;
    Node node;

    Stmt(int l, int c, Node n) : line(l), column(c), node(std::move(n)) {}
};

// ---------- Top level ----------
struct FnDecl {
    std::string name;
    struct Param { std::string name; TypePtr type; };
    std::vector<Param> params;
    TypePtr return_type;   // nullable = unit
    StmtPtr body;
    int line = 0, column = 0;
};

struct Program { std::vector<FnDecl> functions; };

}  // namespace ast