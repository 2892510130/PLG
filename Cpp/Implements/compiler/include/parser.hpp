#pragma once

#include "ast.hpp"

#include <string>
#include <vector>

struct ParseError { int line, column; std::string message; };

struct ParseResult {
    ast::Program program;
    std::vector<ParseError> errors;
    bool ok() const { return errors.empty(); }
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    ParseResult parseProgram();

private:
    const Token& peek() const { return tokens_[pos_]; }
    bool check(TokenKind k) const { return peek().kind == k; }
    bool match(TokenKind k);
    const Token& advance();
    const Token* expect(TokenKind k, std::string_view what);
    void error(const Token& tok, std::string msg);
    void synchronize();

    bool parseItem(ast::Program& out);
    bool parseFnDecl(ast::Program& out);

    ast::TypePtr parseType();

    ast::StmtPtr parseStmt();
    ast::StmtPtr parseLetStmt();
    ast::StmtPtr parseBlock();
    ast::StmtPtr parseIfStmt();
    ast::StmtPtr parseWhileStmt();
    ast::StmtPtr parseReturnStmt();
    ast::StmtPtr parseExprStmt();

    ast::ExprPtr parseExpr();
    ast::ExprPtr parseAssignment();
    ast::ExprPtr parseOr();
    ast::ExprPtr parseAnd();
    ast::ExprPtr parseEquality();
    ast::ExprPtr parseComparison();
    ast::ExprPtr parseTerm();
    ast::ExprPtr parseFactor();
    ast::ExprPtr parseUnary();
    ast::ExprPtr parsePostfix();
    ast::ExprPtr parsePrimary();

    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
    std::vector<ParseError> errors_;
};