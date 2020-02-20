#include "dump_visitor.h"
#include "all_ast.h"
#include <iostream>

namespace c0
{
    bool DumpVisitor::BegVisit(const AST& ast)
    {
        ast.SetUserData(nullptr);
        switch (ast.GetASTType())
        {
        case ASTType::BinaryExpr:
        case ASTType::CastExpr:
        case ASTType::UnaryExpr:
        case ASTType::BraceExpr:
        case ASTType::IdentExpr:
        case ASTType::IntExpr:
        case ASTType::AssignExpr:
        case ASTType::FuncCallExpr:
            _stream << ast.ToString() << std::endl;
            return false;

        case ASTType::EmptyStmt:
            Out() << ast.ToString() << std::endl;
            break;
        case ASTType::BlockStmt:
            break;
        case ASTType::PrintStmt:
        case ASTType::ScanStmt:
            Out() << ast.ToString() << std::endl;
            return false;
        case ASTType::AssignStmt:
            {
                auto& v = dynamic_cast<const AssignStmtAST&>(ast);
                Out() << v.GetName() << " = " << v.GetExpr()->ToString() << ";" << std::endl;
                return false;
            }
        case ASTType::FuncCallStmt:
            Out() << ast.ToString() << std::endl;
            return false;
        case ASTType::IfStmt:
            {
                auto& v = dynamic_cast<const IfStmtAST&>(ast);
                Out() << "if (" << std::to_string(v.GetIfCond()) << ")" << std::endl;

                Out() << "{" << std::endl;
                ++_indent;
                v.GetIFStmt()->Accept(*this);
                --_indent;
                Out() << "}" << std::endl;

                if (nullptr != v.GetElseStmt())
                {
                    Out() << "else" << std::endl;
                    Out() << "{" << std::endl;
                    ++_indent;
                    v.GetElseStmt()->Accept(*this);
                    --_indent;
                    Out() << "}" << std::endl;
                }
                return false;
            }

        case ASTType::SwitchStmt:
            {
                auto& v = dynamic_cast<const SwitchStmtAST&>(ast);
                Out() << "switch (" << std::to_string(v.GetExpr()) << ")" << std::endl;
                Out() << "{" << std::endl;
                for (const auto& stmt : v.GetStmts())
                {
                    if (stmt->GetASTType() == ASTType::LabeledStmt)
                    {
                        stmt->Accept(*this);
                        continue;
                    }

                    Out() << "default:" << std::endl;
                    ++_indent;
                    Out() << "{" << std::endl;
                    ++_indent;
                    stmt->Accept(*this);
                    --_indent;
                    Out() << "}" << std::endl;
                    --_indent;
                }
                Out() << "}" << std::endl;
                return false;
            }
        case ASTType::LabeledStmt:
            {
                auto& v = dynamic_cast<const LabeledStmtAST&>(ast);
                Out() << "cast " << v.GetInt() << ":" << std::endl;
                ++_indent;
                Out() << "{" << std::endl;
                ++_indent;
                v.GetStmt()->Accept(*this);
                --_indent;
                Out() << "}" << std::endl;
                --_indent;
                return false;
            }
            return false;
        case ASTType::WhileStmt:
            {
                auto& v = dynamic_cast<const WhileStmtAST&>(ast);
                Out() << "while (" << std::to_string(v.GetCond()) << ")" << std::endl;
                Out() << "{" << std::endl;
                ++_indent;
                v.GetStmt()->Accept(*this);
                --_indent;
                Out() << "}" << std::endl;
                return false;
            }
        case ASTType::DoStmt:
            {
                auto& v = dynamic_cast<const DoStmtAST&>(ast);
                Out() << "do" << std::endl;
                Out() << "{" << std::endl;
                ++_indent;
                v.GetStmt()->Accept(*this);
                --_indent;
                Out() << "}" << std::endl;
                Out() << "while (" << std::to_string(v.GetCond()) << ");" << std::endl;
                return false;
            }
        case ASTType::ForStmt:
            {
                auto& v = dynamic_cast<const ForStmtAST&>(ast);
                Out() << "for (";
                {
                    bool isFirst = true;
                    for (const auto& expr : v.GetInitExprs())
                    {
                        if (!isFirst)
                            _stream << ", ";
                        _stream << expr->ToString();
                        isFirst = false;
                    }
                }
                _stream << "; " << v.GetCond()->ToString() << "; ";
                {
                    bool isFirst = true;
                    for (const auto& expr : v.GetUpdateExprs())
                    {
                        if (!isFirst)
                            _stream << ", ";
                        _stream << expr->ToString();
                        isFirst = false;
                    }
                }
                _stream << ")" << std::endl;

                Out() << "{" << std::endl;
                ++_indent;
                v.GetBody()->Accept(*this);
                --_indent;
                Out() << "}" << std::endl;
                return false;
            }
        case ASTType::BreakStmt:
        case ASTType::ContinueStmt:
        case ASTType::ReturnStmt:
            Out() << ast.ToString() << std::endl;
            return false;
        case ASTType::VarDecl:
            Out() << ast.ToString() << ";" << std::endl;
            return false;
        case ASTType::FuncDecl:
            {
                _stream << std::endl;
                auto& v = dynamic_cast<const FuncDeclAST&>(ast);
                Out() << std::to_string(v.GetVarType()) << " " << v.GetName() << "(";
                auto params = v.GetParams();
                for (size_t i = 0, N = params.size(); i < N; ++i)
                {
                    if (i > 0)
                        _stream << ", ";
                    _stream << params[i]->ToString();
                }
                Out() << ")" << std::endl;


                Out() << "{" << std::endl;
                ++_indent;
                v.GetBlockStmt()->Accept(*this);
                --_indent;
                Out() << "}" << std::endl;
                return false;
            }
        case ASTType::File:
            break;
        default:
            break;
        }
        return true;
    }

    bool DumpVisitor::EndVisit(const AST& ast)
    {
        switch (ast.GetASTType())
        {
        case ASTType::BinaryExpr:
        case ASTType::CastExpr:
        case ASTType::UnaryExpr:
        case ASTType::BraceExpr:
        case ASTType::IdentExpr:
        case ASTType::IntExpr:
        case ASTType::AssignExpr:
        case ASTType::FuncCallExpr:
        case ASTType::EmptyStmt:
        case ASTType::BlockStmt:
        case ASTType::PrintStmt:
        case ASTType::ScanStmt:
        case ASTType::AssignStmt:
        case ASTType::FuncCallStmt:
        case ASTType::IfStmt:
        case ASTType::SwitchStmt:
        case ASTType::LabeledStmt:
        case ASTType::WhileStmt:
        case ASTType::DoStmt:
        case ASTType::ForStmt:
        case ASTType::BreakStmt:
        case ASTType::ContinueStmt:
        case ASTType::ReturnStmt:
        case ASTType::VarDecl:
        case ASTType::FuncDecl:
        case ASTType::File:
        default:
            break;
        }
        return true;
    }

    str_t DumpVisitor::GetIndent() const
    {
        str_t indent;
        for (int i = 0; i < _indent; ++i)
            indent += "    ";
        return indent;
    }

    std::ostream& DumpVisitor::Out() const
    {
        return _stream << GetIndent();
    }
}