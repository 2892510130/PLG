#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

enum class TokenKind {
    // literals
    Int, Float, String, True, False,
    // identifiers
    Identifier,
    // keywords
    Fn, Let, Mut, If, Else, While, Return, Struct, Import,
    // punctuation
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Comma, Colon, Semicolon, Dot, Arrow,
    // operators
    Assign, Eq, NotEq, Lt, Le, Gt, Ge,
    Plus, Minus, Star, Slash, Percent,
    AndAnd, OrOr, Bang,
    // meta
    Eof, Error,
};

struct Token {
    TokenKind kind = TokenKind::Error;
    std::string_view lexeme;   // slice into source; caller must keep source alive
    int line = 1;
    int column = 1;
    std::string str_value;     // decoded contents for String literals only
};

struct LexError {
    int line;
    int column;
    std::string message;
};

struct LexResult {
    std::vector<Token> tokens;
    std::vector<LexError> errors;
    bool ok() const { return errors.empty(); }
};

class Lexer {
public:
    explicit Lexer(std::string_view source);

    // One-shot: moves errors out. Do not call twice.
    LexResult tokenize();

private:
    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();

    void  skipWhitespaceAndComments();
    Token scanToken();
    Token scanIdentifier();
    Token scanNumber();
    Token scanString();

    Token makeToken(TokenKind kind, std::size_t start, int line, int col) const;
    Token makeError(int line, int col, std::string msg);

    std::string_view src_;
    std::size_t pos_ = 0;
    int line_ = 1;
    int column_ = 1;
    std::vector<LexError> errors_;
};

const char* tokenKindName(TokenKind kind);