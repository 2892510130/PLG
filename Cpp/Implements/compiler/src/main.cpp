#include "ast_dump.hpp"
#include "checker.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interp.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

static std::string readInput(int argc, char** argv) {
    if (argc > 1) {
        std::ifstream f(argv[1], std::ios::binary);
        if (!f) throw std::runtime_error(std::string("cannot open ") + argv[1]);
        return std::string(std::istreambuf_iterator<char>(f), {});
    }
    return std::string(std::istreambuf_iterator<char>(std::cin), {});
}

int main(int argc, char** argv) {
    std::string src;
    try { src = readInput(argc, argv); }
    catch (const std::exception& e) { std::cerr << e.what() << "\n"; return 1; }

    if (src.empty()) src = R"(
fn fib(n: i32) -> i32 {
    if n < 2 {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

fn main() -> i32 {
    print("fib(10) =", fib(10));

    let mut sum = 0;
    let mut i = 1;
    while i <= 10 {
        sum = sum + i;
        i = i + 1;
    }
    print("sum 1..10 =", sum);

    let flag = true;
    if flag {
        print("flag is true");
    } else {
        print("flag is false");
    }

    let pi = 3.14;
    print("pi =", pi, "| two =", 2.0);

    return 0;
}
)";

    // 1. Lex
    Lexer lex(src);
    LexResult lr = lex.tokenize();
    for (auto& e : lr.errors)
        std::cerr << "lex " << e.line << ":" << e.column << ": " << e.message << "\n";
    if (!lr.ok()) return 1;

    // 2. Parse
    Parser parser(std::move(lr.tokens));
    ParseResult pr = parser.parseProgram();
    for (auto& e : pr.errors)
        std::cerr << "parse " << e.line << ":" << e.column << ": " << e.message << "\n";
    if (!pr.ok()) return 1;

    // 3. Semantic
    Checker checker;
    CheckResult cr = checker.check(pr.program);
    for (auto& e : cr.errors)
        std::cerr << "semantic " << e.line << ":" << e.column << ": " << e.message << "\n";
    if (!cr.ok()) return 1;

    // 4. Run
    Interpreter interp;
    try {
        return interp.run(pr.program);
    } catch (const RuntimeError& e) {
        std::cerr << "runtime " << e.line << ":" << e.column << ": " << e.message << "\n";
        return 1;
    }
}