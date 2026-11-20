#include "ast_dump.hpp"

#include <variant>

namespace ast {
namespace {

void indent(std::ostream& os, int n) { for (int i = 0; i < n; ++i) os << "  "; }

std::string opStr(TokenKind k) {
    switch (k) {
        case TokenKind::Plus:   return "+";
        case TokenKind::Minus:  return "-";
        case TokenKind::Star:   return "*";
        case TokenKind::Slash:  return "/";
        case TokenKind::Percent:return "%";
        case TokenKind::Eq:     return "==";
        case TokenKind::NotEq:  return "!=";
        case TokenKind::Lt:     return "<";
        case TokenKind::Le:     return "<=";
        case TokenKind::Gt:     return ">";
        case TokenKind::Ge:     return ">=";
        case TokenKind::AndAnd: return "&&";
        case TokenKind::OrOr:   return "||";
        case TokenKind::Bang:   return "!";
        default:                return "?";
    }
}

void dumpExpr(const Expr& e, std::ostream& os, int depth);
void dumpStmt(const Stmt& s, std::ostream& os, int depth);

void dumpExpr(const Expr& e, std::ostream& os, int depth) {
    indent(os, depth);
    std::visit([&](const auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Expr::IntLit>)
            os << "(int " << node.value << ")\n";
        else if constexpr (std::is_same_v<T, Expr::FloatLit>)
            os << "(float " << node.value << ")\n";
        else if constexpr (std::is_same_v<T, Expr::StringLit>)
            os << "(string \"" << node.value << "\")\n";
        else if constexpr (std::is_same_v<T, Expr::BoolLit>)
            os << "(bool " << (node.value ? "true" : "false") << ")\n";
        else if constexpr (std::is_same_v<T, Expr::Ident>)
            os << "(ident " << node.name << ")\n";
        else if constexpr (std::is_same_v<T, Expr::Unary>) {
            os << "(unary " << opStr(node.op) << "\n";
            dumpExpr(*node.operand, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Expr::Binary>) {
            os << "(binary " << opStr(node.op) << "\n";
            dumpExpr(*node.lhs, os, depth + 1);
            dumpExpr(*node.rhs, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Expr::Assign>) {
            os << "(assign\n";
            dumpExpr(*node.target, os, depth + 1);
            dumpExpr(*node.value,  os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Expr::Call>) {
            os << "(call\n";
            dumpExpr(*node.callee, os, depth + 1);
            for (auto& a : node.args) dumpExpr(*a, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Expr::Index>) {
            os << "(index\n";
            dumpExpr(*node.base,  os, depth + 1);
            dumpExpr(*node.index, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Expr::Field>) {
            os << "(field " << node.name << "\n";
            dumpExpr(*node.base, os, depth + 1);
            indent(os, depth); os << ")\n";
        }
    }, e.node);
}

void dumpStmt(const Stmt& s, std::ostream& os, int depth) {
    std::visit([&](const auto& node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Stmt::Let>) {
            indent(os, depth);
            os << "(let " << (node.is_mut ? "mut " : "") << node.name;
            if (node.type) os << " : " << node.type->toString();
            os << "\n";
            if (node.init) dumpExpr(*node.init, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Stmt::ExprStmt>) {
            indent(os, depth); os << "(expr\n";
            dumpExpr(*node.expr, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Stmt::Block>) {
            indent(os, depth); os << "(block\n";
            for (auto& s : node.stmts) dumpStmt(*s, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Stmt::If>) {
            indent(os, depth); os << "(if\n";
            dumpExpr(*node.cond, os, depth + 1);
            dumpStmt(*node.then_br, os, depth + 1);
            if (node.else_br) dumpStmt(*node.else_br, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Stmt::While>) {
            indent(os, depth); os << "(while\n";
            dumpExpr(*node.cond, os, depth + 1);
            dumpStmt(*node.body, os, depth + 1);
            indent(os, depth); os << ")\n";
        } else if constexpr (std::is_same_v<T, Stmt::Return>) {
            indent(os, depth); os << "(return";
            if (!node.value) { os << ")\n"; return; }
            os << "\n";
            dumpExpr(*node.value, os, depth + 1);
            indent(os, depth); os << ")\n";
        }
    }, s.node);
}

}  // namespace

void dump(const Program& p, std::ostream& os) {
    for (const auto& fn : p.functions) {
        os << "(fn " << fn.name << " (";
        for (size_t i = 0; i < fn.params.size(); ++i) {
            if (i) os << ", ";
            os << fn.params[i].name << ": " << fn.params[i].type->toString();
        }
        os << ")";
        if (fn.return_type) os << " -> " << fn.return_type->toString();
        os << "\n";
        dumpStmt(*fn.body, os, 1);
        os << ")\n";
    }
}

}  // namespace ast