#include "interp.hpp"

#include <cmath>
#include <sstream>
#include <type_traits>

// ---------- Value formatting ----------
std::string Interpreter::valueToString(const Value& v) {
    return std::visit([](const auto& x) -> std::string {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::monostate>) return "()";
        else if constexpr (std::is_same_v<T, int64_t>)    return std::to_string(x);
        else if constexpr (std::is_same_v<T, bool>)       return x ? "true" : "false";
        else if constexpr (std::is_same_v<T, std::string>) return x;
        else {
            std::ostringstream oss;
            oss << x;
            std::string s = oss.str();
            // Make integral-looking floats print as floats: 1.0, not 1
            if (s.find('.') == std::string::npos &&
                s.find('e') == std::string::npos &&
                s.find("inf") == std::string::npos &&
                s.find("nan") == std::string::npos)
                s += ".0";
            return s;
        }
    }, v);
}

// ---------- Variable lookup ----------
Interpreter::Value& Interpreter::lookupVar(const std::string& name, int line, int col) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto f = it->find(name);
        if (f != it->end()) return f->second;
    }
    // Checker should have caught this; defensive.
    throw RuntimeError{line, col, "undefined variable '" + name + "'"};
}

// ---------- Expressions ----------
namespace {

Interpreter::Value arith(TokenKind op, const Interpreter::Value& l,
                         const Interpreter::Value& r, int line, int col) {
    if (auto* a = std::get_if<int64_t>(&l)) {
        if (auto* b = std::get_if<int64_t>(&r)) {
            switch (op) {
                case TokenKind::Plus:    return *a + *b;
                case TokenKind::Minus:   return *a - *b;
                case TokenKind::Star:    return *a * *b;
                case TokenKind::Slash:
                    if (*b == 0) throw RuntimeError{line, col, "integer division by zero"};
                    return *a / *b;
                case TokenKind::Percent:
                    if (*b == 0) throw RuntimeError{line, col, "integer modulo by zero"};
                    return *a % *b;
                default: break;
            }
        }
    }
    if (auto* a = std::get_if<double>(&l)) {
        if (auto* b = std::get_if<double>(&r)) {
            switch (op) {
                case TokenKind::Plus:    return *a + *b;
                case TokenKind::Minus:   return *a - *b;
                case TokenKind::Star:    return *a * *b;
                case TokenKind::Slash:   return *a / *b;   // IEEE
                case TokenKind::Percent: return std::fmod(*a, *b);
                default: break;
            }
        }
    }
    throw RuntimeError{line, col, "internal error: mismatched arithmetic operands"};
}

Interpreter::Value compare(TokenKind op, const Interpreter::Value& l,
                           const Interpreter::Value& r, int line, int col) {
    if (auto* a = std::get_if<int64_t>(&l)) {
        if (auto* b = std::get_if<int64_t>(&r)) {
            switch (op) {
                case TokenKind::Lt: return *a < *b;
                case TokenKind::Le: return *a <= *b;
                case TokenKind::Gt: return *a > *b;
                case TokenKind::Ge: return *a >= *b;
                default: break;
            }
        }
    }
    if (auto* a = std::get_if<double>(&l)) {
        if (auto* b = std::get_if<double>(&r)) {
            switch (op) {
                case TokenKind::Lt: return *a < *b;
                case TokenKind::Le: return *a <= *b;
                case TokenKind::Gt: return *a > *b;
                case TokenKind::Ge: return *a >= *b;
                default: break;
            }
        }
    }
    throw RuntimeError{line, col, "internal error: mismatched comparison operands"};
}

Interpreter::Value equality(TokenKind op, const Interpreter::Value& l,
                            const Interpreter::Value& r, int line, int col) {
    bool eq;
    if (holds_alternative<int64_t>(l) && holds_alternative<int64_t>(r))
        eq = std::get<int64_t>(l) == std::get<int64_t>(r);
    else if (holds_alternative<double>(l) && holds_alternative<double>(r))
        eq = std::get<double>(l) == std::get<double>(r);
    else if (holds_alternative<bool>(l) && holds_alternative<bool>(r))
        eq = std::get<bool>(l) == std::get<bool>(r);
    else if (holds_alternative<std::string>(l) && holds_alternative<std::string>(r))
        eq = std::get<std::string>(l) == std::get<std::string>(r);
    else if (holds_alternative<std::monostate>(l) && holds_alternative<std::monostate>(r))
        eq = true;
    else
        throw RuntimeError{line, col, "internal error: mismatched equality operands"};
    return (op == TokenKind::Eq) ? eq : !eq;
}

}  // namespace

Interpreter::Value Interpreter::evalExpr(ast::Expr& e) {
    return std::visit([&](auto& node) -> Value {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, ast::Expr::IntLit>)
            return node.value;
        else if constexpr (std::is_same_v<T, ast::Expr::FloatLit>)
            return node.value;
        else if constexpr (std::is_same_v<T, ast::Expr::StringLit>)
            return node.value;
        else if constexpr (std::is_same_v<T, ast::Expr::BoolLit>)
            return node.value;
        else if constexpr (std::is_same_v<T, ast::Expr::Ident>)
            return lookupVar(node.name, e.line, e.column);
        else if constexpr (std::is_same_v<T, ast::Expr::Unary>) {
            Value v = evalExpr(*node.operand);
            if (node.op == TokenKind::Minus) {
                if (auto* i = std::get_if<int64_t>(&v)) return -*i;
                if (auto* d = std::get_if<double>(&v))  return -*d;
                throw RuntimeError{e.line, e.column, "internal error: unary '-' on non-numeric"};
            }
            if (auto* b = std::get_if<bool>(&v)) return !*b;
            throw RuntimeError{e.line, e.column, "internal error: '!' on non-bool"};
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Binary>) {
            // Short-circuit before touching RHS
            if (node.op == TokenKind::AndAnd) {
                Value l = evalExpr(*node.lhs);
                if (!std::get<bool>(l)) return false;
                return std::get<bool>(evalExpr(*node.rhs));
            }
            if (node.op == TokenKind::OrOr) {
                Value l = evalExpr(*node.lhs);
                if (std::get<bool>(l)) return true;
                return std::get<bool>(evalExpr(*node.rhs));
            }

            Value l = evalExpr(*node.lhs);
            Value r = evalExpr(*node.rhs);
            switch (node.op) {
                case TokenKind::Plus: case TokenKind::Minus: case TokenKind::Star:
                case TokenKind::Slash: case TokenKind::Percent:
                    return arith(node.op, l, r, e.line, e.column);
                case TokenKind::Lt: case TokenKind::Le:
                case TokenKind::Gt: case TokenKind::Ge:
                    return compare(node.op, l, r, e.line, e.column);
                case TokenKind::Eq: case TokenKind::NotEq:
                    return equality(node.op, l, r, e.line, e.column);
                default:
                    throw RuntimeError{e.line, e.column, "internal error: unknown binary op"};
            }
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Assign>) {
            Value val = evalExpr(*node.value);
            auto* ident = std::get_if<ast::Expr::Ident>(&node.target->node);
            if (!ident)
                throw RuntimeError{e.line, e.column, "internal error: assignment target not an ident"};
            lookupVar(ident->name, e.line, e.column) = val;
            return val;
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Call>) {
            auto* ident = std::get_if<ast::Expr::Ident>(&node.callee->node);
            if (!ident)
                throw RuntimeError{e.line, e.column, "internal error: call target not an ident"};

            std::vector<Value> args;
            args.reserve(node.args.size());
            for (auto& a : node.args) args.push_back(evalExpr(*a));

            if (ident->name == "print") return callPrint(args);

            auto it = functions_.find(ident->name);
            if (it == functions_.end())
                throw RuntimeError{e.line, e.column, "undefined function '" + ident->name + "'"};
            return callFunction(it->second, std::move(args), e.line, e.column);
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Index> ||
                           std::is_same_v<T, ast::Expr::Field>) {
            throw RuntimeError{e.line, e.column, "internal error: unsupported expression at runtime"};
        }
    }, e.node);
}

// ---------- Statements ----------
void Interpreter::execStmt(ast::Stmt& s) {
    std::visit([&](auto& node) {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, ast::Stmt::Let>) {
            Value v = node.init ? evalExpr(*node.init) : Value{};
            scopes_.back()[node.name] = std::move(v);
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::ExprStmt>) {
            evalExpr(*node.expr);
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::Block>) {
            pushScope();
            for (auto& child : node.stmts) execStmt(*child);
            popScope();
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::If>) {
            if (std::get<bool>(evalExpr(*node.cond))) execStmt(*node.then_br);
            else if (node.else_br)                    execStmt(*node.else_br);
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::While>) {
            while (std::get<bool>(evalExpr(*node.cond))) execStmt(*node.body);
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::Return>) {
            Value v = node.value ? evalExpr(*node.value) : Value{};
            throw ReturnSignal{std::move(v)};
        }
    }, s.node);
}

// ---------- Call ----------
Interpreter::Value Interpreter::callFunction(const ast::FnDecl* fn,
                                             std::vector<Value> args,
                                             int line, int col) {
    if (call_depth_ >= kMaxDepth)
        throw RuntimeError{line, col,
            "call stack overflow (depth " + std::to_string(kMaxDepth) + ")"};

    pushScope();
    ++call_depth_;
    struct Guard {
        Interpreter* self;
        ~Guard() { self->popScope(); --self->call_depth_; }
    } guard{this};

    for (size_t i = 0; i < fn->params.size(); ++i)
        scopes_.back()[fn->params[i].name] = std::move(args[i]);

    try {
        execStmt(*fn->body);
        return Value{};
    } catch (ReturnSignal& sig) {
        return std::move(sig.value);
    }
}

Interpreter::Value Interpreter::callPrint(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) out_ << ' ';
        out_ << valueToString(args[i]);
    }
    out_ << '\n' << std::flush;
    return Value{};
}

// ---------- Entry ----------
int Interpreter::run(ast::Program& program) {
    functions_.clear();
    scopes_.clear();
    call_depth_ = 0;

    for (auto& fn : program.functions) functions_[fn.name] = &fn;

    auto it = functions_.find("main");
    if (it == functions_.end())
        throw RuntimeError{0, 0, "no 'main' function"};

    const ast::FnDecl* main_fn = it->second;
    if (!main_fn->params.empty())
        throw RuntimeError{main_fn->line, main_fn->column,
                           "'main' must not take parameters"};

    Value result = callFunction(main_fn, {}, main_fn->line, main_fn->column);

    if (auto* i = std::get_if<int64_t>(&result)) return static_cast<int>(*i);
    if (auto* b = std::get_if<bool>(&result))    return *b ? 1 : 0;
    return 0;
}