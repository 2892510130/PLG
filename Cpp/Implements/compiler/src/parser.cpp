#include "parser.hpp"

#include <charconv>
#include <cstdlib>
#include <cstring>

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

// ---------- Helpers ----------
bool Parser::match(TokenKind k) {
    if (check(k)) { ++pos_; return true; }
    return false;
}

const Token& Parser::advance() {
    // Eof is the sentinel; never advance past it.
    if (pos_ + 1 < tokens_.size()) return tokens_[pos_++];
    return tokens_[pos_];
}

const Token* Parser::expect(TokenKind k, std::string_view what) {
    if (check(k)) return &advance();
    error(peek(), "expected " + std::string(what) + ", got " +
                  std::string(tokenKindName(peek().kind)));
    return nullptr;
}

void Parser::error(const Token& tok, std::string msg) {
    errors_.push_back({tok.line, tok.column, std::move(msg)});
}

void Parser::synchronize() {
    while (!check(TokenKind::Eof)) {
        if (check(TokenKind::Semicolon)) { advance(); return; }
        switch (peek().kind) {
            case TokenKind::Fn: case TokenKind::Let: case TokenKind::If:
            case TokenKind::While: case TokenKind::Return:
            case TokenKind::Struct: case TokenKind::Import:
            case TokenKind::RBrace:
                return;
            default:
                advance();
        }
    }
}

// ---------- Types ----------
ast::TypePtr Parser::parseType() {
    const Token& tok = peek();

    if (check(TokenKind::Identifier)) {
        advance();
        return ast::Type::named(std::string(tok.lexeme));
    }
    if (match(TokenKind::LParen)) {
        if (!expect(TokenKind::RParen, "')' (unit type)")) return nullptr;
        return ast::Type::unit();
    }
    if (match(TokenKind::Fn)) {
        if (!expect(TokenKind::LParen, "'('")) return nullptr;
        std::vector<ast::TypePtr> params;
        if (!check(TokenKind::RParen)) {
            do {
                auto p = parseType();
                if (!p) return nullptr;
                params.push_back(std::move(p));
            } while (match(TokenKind::Comma));
        }
        if (!expect(TokenKind::RParen, "')'")) return nullptr;
        ast::TypePtr ret;
        if (match(TokenKind::Arrow)) {
            ret = parseType();
            if (!ret) return nullptr;
        } else {
            ret = ast::Type::unit();
        }
        return ast::Type::fn(std::move(params), std::move(ret));
    }
    error(tok, "expected type");
    return nullptr;
}

// ---------- Statements ----------
ast::StmtPtr Parser::parseStmt() {
    switch (peek().kind) {
        case TokenKind::Let:    return parseLetStmt();
        case TokenKind::LBrace: return parseBlock();
        case TokenKind::If:     return parseIfStmt();
        case TokenKind::While:  return parseWhileStmt();
        case TokenKind::Return: return parseReturnStmt();
        default:                return parseExprStmt();
    }
}

ast::StmtPtr Parser::parseLetStmt() {
    const Token& letTok = advance();
    bool is_mut = match(TokenKind::Mut);

    const Token* nameTok = expect(TokenKind::Identifier, "variable name");
    if (!nameTok) return nullptr;

    ast::TypePtr type;
    if (match(TokenKind::Colon)) {
        type = parseType();
        if (!type) return nullptr;
    }

    ast::ExprPtr init;
    if (match(TokenKind::Assign)) {
        init = parseExpr();
        if (!init) return nullptr;
    }

    if (!expect(TokenKind::Semicolon, "';'")) return nullptr;

    ast::Stmt::Let let;
    let.name   = std::string(nameTok->lexeme);
    let.is_mut = is_mut;
    let.type   = std::move(type);
    let.init   = std::move(init);
    return std::make_unique<ast::Stmt>(letTok.line, letTok.column, std::move(let));
}

ast::StmtPtr Parser::parseBlock() {
    const Token& lbrace = advance();
    std::vector<ast::StmtPtr> stmts;
    while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
        std::size_t before = pos_;
        auto s = parseStmt();
        if (s) stmts.push_back(std::move(s));
        else {
            synchronize();
            if (pos_ == before) advance();   // force progress
        }
    }
    if (!expect(TokenKind::RBrace, "'}'")) return nullptr;

    ast::Stmt::Block block;
    block.stmts = std::move(stmts);
    return std::make_unique<ast::Stmt>(lbrace.line, lbrace.column, std::move(block));
}

ast::StmtPtr Parser::parseIfStmt() {
    const Token& ifTok = advance();
    auto cond = parseExpr();
    if (!cond) return nullptr;
    auto then_br = parseBlock();
    if (!then_br) return nullptr;

    ast::StmtPtr else_br;
    if (match(TokenKind::Else)) {
        else_br = check(TokenKind::If) ? parseIfStmt() : parseBlock();
        if (!else_br) return nullptr;
    }

    ast::Stmt::If s;
    s.cond    = std::move(cond);
    s.then_br = std::move(then_br);
    s.else_br = std::move(else_br);
    return std::make_unique<ast::Stmt>(ifTok.line, ifTok.column, std::move(s));
}

ast::StmtPtr Parser::parseWhileStmt() {
    const Token& whileTok = advance();
    auto cond = parseExpr();
    if (!cond) return nullptr;
    auto body = parseBlock();
    if (!body) return nullptr;

    ast::Stmt::While w;
    w.cond = std::move(cond);
    w.body = std::move(body);
    return std::make_unique<ast::Stmt>(whileTok.line, whileTok.column, std::move(w));
}

ast::StmtPtr Parser::parseReturnStmt() {
    const Token& retTok = advance();
    ast::ExprPtr value;
    if (!check(TokenKind::Semicolon)) {
        value = parseExpr();
        if (!value) return nullptr;
    }
    if (!expect(TokenKind::Semicolon, "';'")) return nullptr;

    ast::Stmt::Return r;
    r.value = std::move(value);
    return std::make_unique<ast::Stmt>(retTok.line, retTok.column, std::move(r));
}

ast::StmtPtr Parser::parseExprStmt() {
    auto expr = parseExpr();
    if (!expr) return nullptr;
    int line = expr->line, col = expr->column;
    if (!expect(TokenKind::Semicolon, "';'")) return nullptr;

    ast::Stmt::ExprStmt es;
    es.expr = std::move(expr);
    return std::make_unique<ast::Stmt>(line, col, std::move(es));
}

// ---------- Expressions (precedence climbing) ----------
ast::ExprPtr Parser::parseExpr() { return parseAssignment(); }

ast::ExprPtr Parser::parseAssignment() {
    auto lhs = parseOr();
    if (!lhs) return nullptr;
    if (match(TokenKind::Assign)) {
        int line = lhs->line, col = lhs->column;
        auto rhs = parseAssignment();   // right-assoc
        if (!rhs) return nullptr;
        ast::Expr::Assign a;
        a.target = std::move(lhs);
        a.value  = std::move(rhs);
        return std::make_unique<ast::Expr>(line, col, std::move(a));
    }
    return lhs;
}

#define BINARY_CHAIN(NAME, NEXT, ...)                                        \
    ast::ExprPtr Parser::NAME() {                                           \
        auto lhs = NEXT();                                                  \
        if (!lhs) return nullptr;                                           \
        for (;;) {                                                          \
            TokenKind op;                                                   \
            bool matched = false;                                           \
            for (TokenKind cand : { __VA_ARGS__ }) {                        \
                if (check(cand)) { op = cand; matched = true; break; }      \
            }                                                               \
            if (!matched) break;                                            \
            advance();                                                      \
            int line = lhs->line, col = lhs->column;                        \
            auto rhs = NEXT();                                              \
            if (!rhs) return nullptr;                                       \
            ast::Expr::Binary b;                                            \
            b.op  = op;                                                     \
            b.lhs = std::move(lhs);                                         \
            b.rhs = std::move(rhs);                                         \
            lhs = std::make_unique<ast::Expr>(line, col, std::move(b));     \
        }                                                                   \
        return lhs;                                                         \
    }

BINARY_CHAIN(parseOr,         parseAnd,        TokenKind::OrOr)
BINARY_CHAIN(parseAnd,        parseEquality,   TokenKind::AndAnd)
BINARY_CHAIN(parseEquality,   parseComparison, TokenKind::Eq, TokenKind::NotEq)
BINARY_CHAIN(parseComparison, parseTerm,       TokenKind::Lt, TokenKind::Le,
                                               TokenKind::Gt, TokenKind::Ge)
BINARY_CHAIN(parseTerm,       parseFactor,     TokenKind::Plus, TokenKind::Minus)
BINARY_CHAIN(parseFactor,     parseUnary,      TokenKind::Star,
                                               TokenKind::Slash, TokenKind::Percent)
#undef BINARY_CHAIN

ast::ExprPtr Parser::parseUnary() {
    if (check(TokenKind::Minus) || check(TokenKind::Bang)) {
        const Token& opTok = advance();
        auto operand = parseUnary();
        if (!operand) return nullptr;
        ast::Expr::Unary u;
        u.op = opTok.kind;
        u.operand = std::move(operand);
        return std::make_unique<ast::Expr>(opTok.line, opTok.column, std::move(u));
    }
    return parsePostfix();
}

ast::ExprPtr Parser::parsePostfix() {
    auto expr = parsePrimary();
    if (!expr) return nullptr;
    for (;;) {
        if (match(TokenKind::LParen)) {
            int line = expr->line, col = expr->column;
            std::vector<ast::ExprPtr> args;
            if (!check(TokenKind::RParen)) {
                do {
                    auto a = parseExpr();
                    if (!a) return nullptr;
                    args.push_back(std::move(a));
                } while (match(TokenKind::Comma));
            }
            if (!expect(TokenKind::RParen, "')'")) return nullptr;
            ast::Expr::Call c;
            c.callee = std::move(expr);
            c.args   = std::move(args);
            expr = std::make_unique<ast::Expr>(line, col, std::move(c));
        } else if (match(TokenKind::Dot)) {
            int line = expr->line, col = expr->column;
            const Token* n = expect(TokenKind::Identifier, "field name");
            if (!n) return nullptr;
            ast::Expr::Field f;
            f.base = std::move(expr);
            f.name = std::string(n->lexeme);
            expr = std::make_unique<ast::Expr>(line, col, std::move(f));
        } else if (match(TokenKind::LBracket)) {
            int line = expr->line, col = expr->column;
            auto idx = parseExpr();
            if (!idx) return nullptr;
            if (!expect(TokenKind::RBracket, "']'")) return nullptr;
            ast::Expr::Index i;
            i.base  = std::move(expr);
            i.index = std::move(idx);
            expr = std::make_unique<ast::Expr>(line, col, std::move(i));
        } else {
            break;
        }
    }
    return expr;
}

ast::ExprPtr Parser::parsePrimary() {
    const Token& tok = peek();

    if (check(TokenKind::Int)) {
        advance();
        int64_t v = 0;
        auto [_, ec] = std::from_chars(tok.lexeme.data(),
                                      tok.lexeme.data() + tok.lexeme.size(), v);
        if (ec != std::errc()) { error(tok, "integer literal out of range"); v = 0; }
        ast::Expr::IntLit n{v};
        return std::make_unique<ast::Expr>(tok.line, tok.column, std::move(n));
    }
    if (check(TokenKind::Float)) {
        advance();
        std::string buf(tok.lexeme);
        double v = std::strtod(buf.c_str(), nullptr);
        ast::Expr::FloatLit n{v};
        return std::make_unique<ast::Expr>(tok.line, tok.column, std::move(n));
    }
    if (check(TokenKind::String)) {
        advance();
        ast::Expr::StringLit n{tok.str_value};
        return std::make_unique<ast::Expr>(tok.line, tok.column, std::move(n));
    }
    if (check(TokenKind::True) || check(TokenKind::False)) {
        bool v = check(TokenKind::True);
        advance();
        ast::Expr::BoolLit n{v};
        return std::make_unique<ast::Expr>(tok.line, tok.column, std::move(n));
    }
    if (check(TokenKind::Identifier)) {
        advance();
        ast::Expr::Ident n{std::string(tok.lexeme)};
        return std::make_unique<ast::Expr>(tok.line, tok.column, std::move(n));
    }
    if (match(TokenKind::LParen)) {
        auto inner = parseExpr();
        if (!inner) return nullptr;
        if (!expect(TokenKind::RParen, "')'")) return nullptr;
        return inner;
    }
    error(tok, "expected expression");
    return nullptr;
}

// ---------- Top level ----------
bool Parser::parseItem(ast::Program& out) {
    if (check(TokenKind::Fn)) return parseFnDecl(out);
    error(peek(), "expected 'fn' at top level");
    return false;
}

bool Parser::parseFnDecl(ast::Program& out) {
    const Token& fnTok = advance();   // 'fn'
    const Token* nameTok = expect(TokenKind::Identifier, "function name");
    if (!nameTok) return false;

    ast::FnDecl fn;
    fn.name   = std::string(nameTok->lexeme);
    fn.line   = fnTok.line;
    fn.column = fnTok.column;

    if (!expect(TokenKind::LParen, "'('")) return false;
    if (!check(TokenKind::RParen)) {
        do {
            const Token* pn = expect(TokenKind::Identifier, "parameter name");
            if (!pn) return false;
            if (!expect(TokenKind::Colon, "':' after parameter name")) return false;
            auto pt = parseType();
            if (!pt) return false;
            fn.params.push_back({std::string(pn->lexeme), std::move(pt)});
        } while (match(TokenKind::Comma));
    }
    if (!expect(TokenKind::RParen, "')'")) return false;

    if (match(TokenKind::Arrow)) {
        fn.return_type = parseType();
        if (!fn.return_type) return false;
    }
    fn.body = parseBlock();
    if (!fn.body) return false;

    out.functions.push_back(std::move(fn));
    return true;
}

ParseResult Parser::parseProgram() {
    ast::Program prog;
    while (!check(TokenKind::Eof)) {
        std::size_t before = pos_;
        if (!parseItem(prog)) {
            synchronize();
            if (pos_ == before) advance();   // guarantee progress
        }
    }
    return ParseResult{std::move(prog), std::move(errors_)};
}