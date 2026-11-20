#include "checker.hpp"

#include <type_traits>
#include <variant>

namespace {

using ast::Type;

ast::TypePtr cloneType(const Type& t) {
    auto r = std::make_unique<Type>();
    r->kind = t.kind;
    r->name = t.name;
    for (auto& p : t.params) r->params.push_back(cloneType(*p));
    if (t.ret) r->ret = cloneType(*t.ret);
    return r;
}

bool isInteger(const Type& t) {
    return t.kind == Type::Kind::Named &&
           (t.name == "i32" || t.name == "i64" ||
            t.name == "u32" || t.name == "u64");
}
bool isFloat(const Type& t) {
    return t.kind == Type::Kind::Named && (t.name == "f32" || t.name == "f64");
}
bool isNumeric(const Type& t) { return isInteger(t) || isFloat(t); }
bool isBool(const Type& t)    { return t.kind == Type::Kind::Named && t.name == "bool"; }

bool isValidType(const Type& t) {
    switch (t.kind) {
        case Type::Kind::Unit: return true;
        case Type::Kind::Named:
            return isInteger(t) || isFloat(t) || t.name == "bool" || t.name == "string";
        case Type::Kind::Fn:
            for (auto& p : t.params) if (!isValidType(*p)) return false;
            if (t.ret && !isValidType(*t.ret)) return false;
            return true;
    }
    return false;
}

bool typesEqual(const Type& a, const Type& b) {
    if (a.kind != b.kind) return false;
    switch (a.kind) {
        case Type::Kind::Named: return a.name == b.name;
        case Type::Kind::Unit:  return true;
        case Type::Kind::Fn: {
            if (a.params.size() != b.params.size()) return false;
            for (size_t i = 0; i < a.params.size(); ++i)
                if (!typesEqual(*a.params[i], *b.params[i])) return false;
            const bool a_has = a.ret != nullptr, b_has = b.ret != nullptr;
            if (a_has != b_has) return false;
            return !a_has || typesEqual(*a.ret, *b.ret);
        }
    }
    return false;
}

// Conservative: does `stmt` always return on every path through it?
bool alwaysReturns(const ast::Stmt& stmt) {
    return std::visit([](const auto& node) -> bool {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, ast::Stmt::Return>) return true;
        else if constexpr (std::is_same_v<T, ast::Stmt::Block>) {
            for (auto& s : node.stmts) if (alwaysReturns(*s)) return true;
            return false;
        } else if constexpr (std::is_same_v<T, ast::Stmt::If>) {
            if (!node.else_br) return false;
            return alwaysReturns(*node.then_br) && alwaysReturns(*node.else_br);
        } else return false;
    }, stmt.node);
}

}  // namespace

// ---------- Infrastructure ----------
void Checker::error(int line, int col, std::string msg) {
    errors_.push_back({line, col, std::move(msg)});
}
void Checker::pushScope() { scopes_.emplace_back(); }
void Checker::popScope()  { scopes_.pop_back(); }

Checker::VarInfo* Checker::lookupVar(const std::string& name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto f = it->find(name);
        if (f != it->end()) return &f->second;
    }
    return nullptr;
}

bool Checker::declareVar(const std::string& name, VarInfo info, int line, int col) {
    auto [_, ok] = scopes_.back().emplace(name, std::move(info));
    if (!ok) error(line, col, "variable '" + name + "' already declared in this scope");
    return ok;
}

// ---------- Top-level ----------
CheckResult Checker::check(ast::Program& program) {
    collectFunctions(program);
    for (auto& fn : program.functions) checkFn(fn);
    return CheckResult{std::move(errors_)};
}

void Checker::collectFunctions(const ast::Program& program) {
    for (auto& fn : program.functions) {
        auto [_, ok] = functions_.emplace(fn.name, FnInfo{&fn});
        if (!ok) error(fn.line, fn.column, "function '" + fn.name + "' already defined");
    }
}

void Checker::checkFn(const ast::FnDecl& fn) {
    for (auto& p : fn.params)
        if (!isValidType(*p.type)) {
            error(fn.line, fn.column,
                  "parameter '" + p.name + "' has unknown type '" + p.type->toString() + "'");
            return;
        }
    if (fn.return_type && !isValidType(*fn.return_type)) {
        error(fn.line, fn.column,
              "function '" + fn.name + "' has unknown return type '" +
              fn.return_type->toString() + "'");
        return;
    }

    current_fn_ = &fn;
    pushScope();
    for (auto& p : fn.params)
        declareVar(p.name, VarInfo{cloneType(*p.type), /*is_mut=*/true},
                   fn.line, fn.column);

    checkStmt(*fn.body);

    if (fn.return_type && !alwaysReturns(*fn.body))
        error(fn.line, fn.column,
              "function '" + fn.name + "' returns " + fn.return_type->toString() +
              " but may reach end without returning");

    popScope();
    current_fn_ = nullptr;
}

// ---------- Statements ----------
void Checker::checkStmt(const ast::Stmt& stmt) {
    std::visit([&](const auto& node) {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, ast::Stmt::Let>) {
            if (node.type && !isValidType(*node.type)) {
                error(stmt.line, stmt.column,
                      "unknown type '" + node.type->toString() + "'");
                return;
            }
            ast::TypePtr type = node.type ? cloneType(*node.type) : nullptr;

            if (node.init) {
                if (type) coerce(*node.init, *type, "initializer");
                else {
                    type = checkExpr(*node.init);
                    if (!type) return;   // error already reported
                }
            } else if (!type) {
                error(stmt.line, stmt.column,
                      "'let' requires a type annotation or initializer");
                return;
            }
            declareVar(node.name, VarInfo{std::move(type), node.is_mut},
                       stmt.line, stmt.column);
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::ExprStmt>) {
            checkExpr(*node.expr);
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::Block>) {
            pushScope();
            for (auto& s : node.stmts) checkStmt(*s);
            popScope();
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::If>) {
            auto ct = checkExpr(*node.cond);
            if (ct && !isBool(*ct))
                error(node.cond->line, node.cond->column,
                      "if condition must be 'bool', got '" + ct->toString() + "'");
            checkStmt(*node.then_br);
            if (node.else_br) checkStmt(*node.else_br);
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::While>) {
            auto ct = checkExpr(*node.cond);
            if (ct && !isBool(*ct))
                error(node.cond->line, node.cond->column,
                      "while condition must be 'bool', got '" + ct->toString() + "'");
            checkStmt(*node.body);
        }
        else if constexpr (std::is_same_v<T, ast::Stmt::Return>) {
            if (!current_fn_) return;   // shouldn't happen; defensive
            if (node.value) {
                if (!current_fn_->return_type) {
                    error(stmt.line, stmt.column,
                          "cannot return a value from function returning ()");
                } else {
                    coerce(*node.value, *current_fn_->return_type, "return");
                }
            } else if (current_fn_->return_type) {
                error(stmt.line, stmt.column,
                      "return without a value in function returning " +
                      current_fn_->return_type->toString());
            }
        }
    }, stmt.node);
}

// ---------- Expressions ----------
ast::TypePtr Checker::checkExpr(ast::Expr& e, const ast::Type* expected) {
    ast::TypePtr result = std::visit([&](auto& node) -> ast::TypePtr {
        using T = std::decay_t<decltype(node)>;

        if constexpr (std::is_same_v<T, ast::Expr::IntLit>) {
            if (expected && isInteger(*expected)) return cloneType(*expected);
            if (expected && isFloat(*expected))   return cloneType(*expected);
            return Type::named("i32");
        }
        else if constexpr (std::is_same_v<T, ast::Expr::FloatLit>) {
            if (expected && isFloat(*expected)) return cloneType(*expected);
            return Type::named("f64");
        }
        else if constexpr (std::is_same_v<T, ast::Expr::StringLit>) {
            return Type::named("string");
        }
        else if constexpr (std::is_same_v<T, ast::Expr::BoolLit>) {
            return Type::named("bool");
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Ident>) {
            if (auto* info = lookupVar(node.name)) return cloneType(*info->type);
            if (functions_.count(node.name))
                error(e.line, e.column,
                      "'" + node.name + "' is a function; use '" + node.name + "(...)' to call it");
            else
                error(e.line, e.column, "undefined variable '" + node.name + "'");
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Unary>) {
            const Type* hint = (expected && isNumeric(*expected)) ? expected : nullptr;
            auto ot = checkExpr(*node.operand, hint);
            if (!ot) return nullptr;
            if (node.op == TokenKind::Minus) {
                if (!isNumeric(*ot)) {
                    error(e.line, e.column, "unary '-' requires numeric operand");
                    return nullptr;
                }
                return ot;
            }
            // Bang
            if (!isBool(*ot)) {
                error(e.line, e.column, "unary '!' requires bool operand");
                return nullptr;
            }
            return Type::named("bool");
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Binary>) {
            const bool is_arith = node.op == TokenKind::Plus  || node.op == TokenKind::Minus ||
                                  node.op == TokenKind::Star  || node.op == TokenKind::Slash ||
                                  node.op == TokenKind::Percent;
            const Type* hint = (is_arith && expected && isNumeric(*expected)) ? expected : nullptr;

            auto lt = checkExpr(*node.lhs, hint);
            auto rt = checkExpr(*node.rhs, hint);
            if (!lt || !rt) return nullptr;

            if (is_arith) {
                if (!isNumeric(*lt) || !isNumeric(*rt)) {
                    error(e.line, e.column, "arithmetic operator requires numeric operands");
                    return nullptr;
                }
                if (!typesEqual(*lt, *rt)) {
                    error(e.line, e.column,
                          "operands must have the same type: '" +
                          lt->toString() + "' vs '" + rt->toString() + "'");
                    return nullptr;
                }
                return lt;
            }
            if (node.op == TokenKind::Lt || node.op == TokenKind::Le ||
                node.op == TokenKind::Gt || node.op == TokenKind::Ge) {
                if (!isNumeric(*lt) || !isNumeric(*rt)) {
                    error(e.line, e.column, "comparison requires numeric operands");
                    return nullptr;
                }
                if (!typesEqual(*lt, *rt)) {
                    error(e.line, e.column, "comparison operands must have the same type");
                    return nullptr;
                }
                return Type::named("bool");
            }
            if (node.op == TokenKind::Eq || node.op == TokenKind::NotEq) {
                if (!typesEqual(*lt, *rt)) {
                    error(e.line, e.column,
                          "equality operands must have the same type: '" +
                          lt->toString() + "' vs '" + rt->toString() + "'");
                    return nullptr;
                }
                return Type::named("bool");
            }
            // AndAnd / OrOr
            if (!isBool(*lt) || !isBool(*rt)) {
                error(e.line, e.column,
                      "logical operator requires 'bool' operands");
                return nullptr;
            }
            return Type::named("bool");
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Assign>) {
            auto* ident = std::get_if<ast::Expr::Ident>(&node.target->node);
            if (!ident) {
                error(e.line, e.column, "invalid assignment target");
                return nullptr;
            }
            auto* info = lookupVar(ident->name);
            if (!info) {
                error(e.line, e.column, "undefined variable '" + ident->name + "'");
                return nullptr;
            }
            if (!info->is_mut) {
                error(e.line, e.column,
                      "cannot assign to immutable variable '" + ident->name + "'");
                return nullptr;
            }
            coerce(*node.value, *info->type, "assignment");
            node.target->inferred_type = cloneType(*info->type);
            return cloneType(*info->type);
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Call>) {
            auto* ident = std::get_if<ast::Expr::Ident>(&node.callee->node);
            if (!ident) {
                error(e.line, e.column, "call target must be a function name (v1)");
                return nullptr;
            }

            if (ident->name == "print") {
                for (auto& a : node.args) checkExpr(*a);
                return Type::unit();
            }
            
            auto it = functions_.find(ident->name);
            if (it == functions_.end()) {
                error(e.line, e.column, "undefined function '" + ident->name + "'");
                return nullptr;
            }
            const auto& fn = *it->second.decl;

            if (node.args.size() != fn.params.size()) {
                error(e.line, e.column,
                      "function '" + ident->name + "' expects " +
                      std::to_string(fn.params.size()) + " argument(s), got " +
                      std::to_string(node.args.size()));
                return nullptr;
            }
            for (size_t i = 0; i < node.args.size(); ++i)
                coerce(*node.args[i], *fn.params[i].type, "argument");

            return fn.return_type ? cloneType(*fn.return_type) : Type::unit();
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Index>) {
            error(e.line, e.column, "array indexing is not supported yet");
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, ast::Expr::Field>) {
            error(e.line, e.column, "struct field access is not supported yet");
            return nullptr;
        }
    }, e.node);

    if (!result) return nullptr;
    e.inferred_type = cloneType(*result);
    return result;
}

bool Checker::coerce(ast::Expr& expr, const ast::Type& target, const char* context) {
    auto inferred = checkExpr(expr, &target);
    if (!inferred) return false;   // error already reported
    if (!typesEqual(*inferred, target)) {
        error(expr.line, expr.column,
              std::string(context) + ": expected '" + target.toString() +
              "', got '" + inferred->toString() + "'");
        return false;
    }
    return true;
}