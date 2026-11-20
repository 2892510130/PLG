#include "lexer.hpp"

#include <cctype>
#include <unordered_map>

namespace {

bool isIdentStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}
bool isIdentCont(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}
bool isDigit(char c) { return c >= '0' && c <= '9'; }

const std::unordered_map<std::string_view, TokenKind>& keywords() {
    static const std::unordered_map<std::string_view, TokenKind> kKeywords = {
        {"fn",     TokenKind::Fn},
        {"let",    TokenKind::Let},
        {"mut",    TokenKind::Mut},
        {"if",     TokenKind::If},
        {"else",   TokenKind::Else},
        {"while",  TokenKind::While},
        {"return", TokenKind::Return},
        {"struct", TokenKind::Struct},
        {"import", TokenKind::Import},
        {"true",   TokenKind::True},
        {"false",  TokenKind::False},
    };
    return kKeywords;
}

} // namespace

Lexer::Lexer(std::string_view source) : src_(source) {}

bool Lexer::isAtEnd() const { return pos_ >= src_.size(); }
char Lexer::peek()     const { return isAtEnd() ? '\0' : src_[pos_]; }
char Lexer::peekNext() const { return (pos_ + 1 < src_.size()) ? src_[pos_ + 1] : '\0'; }

char Lexer::advance() {
    char c = src_[pos_++];
    if (c == '\n') { ++line_; column_ = 1; }
    else           { ++column_; }
    return c;
}

Token Lexer::makeToken(TokenKind kind, std::size_t start, int line, int col) const {
    return Token{kind, src_.substr(start, pos_ - start), line, col};
}

Token Lexer::makeError(int line, int col, std::string msg) {
    errors_.push_back({line, col, std::move(msg)});
    return Token{TokenKind::Error, {}, line, col};
}

void Lexer::skipWhitespaceAndComments() {
    for (;;) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { advance(); continue; }

        // line comment
        if (c == '/' && peekNext() == '/') {
            while (!isAtEnd() && peek() != '\n') advance();
            continue;
        }
        // block comment (non-nested, like C)
        if (c == '/' && peekNext() == '*') {
            int startLine = line_, startCol = column_;
            advance(); advance();
            bool closed = false;
            while (!isAtEnd()) {
                if (peek() == '*' && peekNext() == '/') {
                    advance(); advance(); closed = true; break;
                }
                advance();
            }
            if (!closed)
                errors_.push_back({startLine, startCol, "unterminated block comment"});
            continue;
        }
        break;
    }
}

Token Lexer::scanIdentifier() {
    int line = line_, col = column_;
    std::size_t start = pos_;
    while (!isAtEnd() && isIdentCont(peek())) advance();

    std::string_view text = src_.substr(start, pos_ - start);
    auto& kw = keywords();
    auto it = kw.find(text);
    TokenKind kind = (it != kw.end()) ? it->second : TokenKind::Identifier;
    return Token{kind, text, line, col};
}

Token Lexer::scanNumber() {
    int line = line_, col = column_;
    std::size_t start = pos_;
    while (!isAtEnd() && isDigit(peek())) advance();

    bool isFloat = false;
    // Require a digit after '.'; "1." lexes as Int(1) Dot, which the parser rejects.
    if (!isAtEnd() && peek() == '.' && isDigit(peekNext())) {
        isFloat = true;
        advance();  // '.'
        while (!isAtEnd() && isDigit(peek())) advance();
    }
    return Token{
        isFloat ? TokenKind::Float : TokenKind::Int,
        src_.substr(start, pos_ - start),
        line, col
    };
}

Token Lexer::scanString() {
    int line = line_, col = column_;
    std::size_t start = pos_;
    advance();  // opening '"'

    std::string value;
    while (!isAtEnd()) {
        char c = advance();
        if (c == '"') {
            Token tok{TokenKind::String, src_.substr(start, pos_ - start), line, col};
            tok.str_value = std::move(value);
            return tok;
        }
        if (c == '\n') {  // v1: no multi-line strings
            errors_.push_back({line, col, "unterminated string literal"});
            return Token{TokenKind::Error, src_.substr(start, pos_ - start), line, col};
        }
        if (c == '\\') {
            if (isAtEnd()) break;
            char e = advance();
            switch (e) {
                case 'n':  value += '\n'; break;
                case 't':  value += '\t'; break;
                case 'r':  value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"':  value += '"';  break;
                case '0':  value += '\0'; break;
                default:
                    errors_.push_back({line, col,
                        std::string("unknown escape sequence '\\") + e + "'"});
                    value += e;  // keep scanning; don't cascade
                    break;
            }
        } else {
            value += c;
        }
    }
    errors_.push_back({line, col, "unterminated string literal"});
    return Token{TokenKind::Error, src_.substr(start, pos_ - start), line, col};
}

Token Lexer::scanToken() {
    int line = line_, col = column_;
    std::size_t start = pos_;
    char c = peek();

    if (isIdentStart(c)) return scanIdentifier();
    if (isDigit(c))      return scanNumber();
    if (c == '"')        return scanString();

    advance();
    switch (c) {
        case '(': return makeToken(TokenKind::LParen,    start, line, col);
        case ')': return makeToken(TokenKind::RParen,    start, line, col);
        case '{': return makeToken(TokenKind::LBrace,    start, line, col);
        case '}': return makeToken(TokenKind::RBrace,    start, line, col);
        case '[': return makeToken(TokenKind::LBracket,  start, line, col);
        case ']': return makeToken(TokenKind::RBracket,  start, line, col);
        case ',': return makeToken(TokenKind::Comma,     start, line, col);
        case ';': return makeToken(TokenKind::Semicolon, start, line, col);
        case '.': return makeToken(TokenKind::Dot,       start, line, col);
        case ':': return makeToken(TokenKind::Colon,     start, line, col);

        case '=':
            if (peek() == '=') { advance(); return makeToken(TokenKind::Eq,     start, line, col); }
            return makeToken(TokenKind::Assign, start, line, col);
        case '!':
            if (peek() == '=') { advance(); return makeToken(TokenKind::NotEq,  start, line, col); }
            return makeToken(TokenKind::Bang, start, line, col);
        case '<':
            if (peek() == '=') { advance(); return makeToken(TokenKind::Le,     start, line, col); }
            return makeToken(TokenKind::Lt, start, line, col);
        case '>':
            if (peek() == '=') { advance(); return makeToken(TokenKind::Ge,     start, line, col); }
            return makeToken(TokenKind::Gt, start, line, col);
        case '+': return makeToken(TokenKind::Plus, start, line, col);
        case '-':
            if (peek() == '>') { advance(); return makeToken(TokenKind::Arrow,  start, line, col); }
            return makeToken(TokenKind::Minus, start, line, col);
        case '*': return makeToken(TokenKind::Star,    start, line, col);
        case '/': return makeToken(TokenKind::Slash,   start, line, col);
        case '%': return makeToken(TokenKind::Percent, start, line, col);
        case '&':
            if (peek() == '&') { advance(); return makeToken(TokenKind::AndAnd, start, line, col); }
            return makeError(line, col, "expected '&&'");
        case '|':
            if (peek() == '|') { advance(); return makeToken(TokenKind::OrOr,   start, line, col); }
            return makeError(line, col, "expected '||'");
    }
    return makeError(line, col, std::string("unexpected character '") + c + "'");
}

LexResult Lexer::tokenize() {
    std::vector<Token> tokens;
    for (;;) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;
        Token t = scanToken();
        if (t.kind == TokenKind::Error) continue;   // already recorded
        tokens.push_back(std::move(t));
    }
    tokens.push_back(Token{TokenKind::Eof, {}, line_, column_});
    return LexResult{std::move(tokens), std::move(errors_)};
}

const char* tokenKindName(TokenKind kind) {
    switch (kind) {
        case TokenKind::Int:        return "Int";
        case TokenKind::Float:      return "Float";
        case TokenKind::String:     return "String";
        case TokenKind::True:       return "True";
        case TokenKind::False:      return "False";
        case TokenKind::Identifier: return "Identifier";
        case TokenKind::Fn:         return "Fn";
        case TokenKind::Let:        return "Let";
        case TokenKind::Mut:        return "Mut";
        case TokenKind::If:         return "If";
        case TokenKind::Else:       return "Else";
        case TokenKind::While:      return "While";
        case TokenKind::Return:     return "Return";
        case TokenKind::Struct:     return "Struct";
        case TokenKind::Import:     return "Import";
        case TokenKind::LParen:     return "LParen";
        case TokenKind::RParen:     return "RParen";
        case TokenKind::LBrace:     return "LBrace";
        case TokenKind::RBrace:     return "RBrace";
        case TokenKind::LBracket:   return "LBracket";
        case TokenKind::RBracket:   return "RBracket";
        case TokenKind::Comma:      return "Comma";
        case TokenKind::Colon:      return "Colon";
        case TokenKind::Semicolon:  return "Semicolon";
        case TokenKind::Dot:        return "Dot";
        case TokenKind::Arrow:      return "Arrow";
        case TokenKind::Assign:     return "Assign";
        case TokenKind::Eq:         return "Eq";
        case TokenKind::NotEq:      return "NotEq";
        case TokenKind::Lt:         return "Lt";
        case TokenKind::Le:         return "Le";
        case TokenKind::Gt:         return "Gt";
        case TokenKind::Ge:         return "Ge";
        case TokenKind::Plus:       return "Plus";
        case TokenKind::Minus:      return "Minus";
        case TokenKind::Star:       return "Star";
        case TokenKind::Slash:      return "Slash";
        case TokenKind::Percent:    return "Percent";
        case TokenKind::AndAnd:     return "AndAnd";
        case TokenKind::OrOr:       return "OrOr";
        case TokenKind::Bang:       return "Bang";
        case TokenKind::Eof:        return "Eof";
        case TokenKind::Error:      return "Error";
    }
    return "?";
}