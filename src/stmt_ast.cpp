#include "all_ast.h"


namespace c0
{
    std::string EmptyStmtAST::ToString() const
    {
        return ";";
    }

    bool EmptyStmtAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    SymbolType EmptyStmtAST::GetSymbolType(const str_t& s, bool recusive) const
    {
        return DefaultGetSymbolTypeImpl(s, recusive);
    }

    ASTPtr EmptyStmtAST::GetSymbol(const str_t& s, bool recusive) const
    {
        return DefaultGetSymbolImpl(s, recusive);
    }

    std::string BlockStmtAST::ToString() const
    {
        std::string s = "{\n";
        for (const auto& var : _vars)
            s += var->ToString() + ";\n";
        for (const auto& stmt : _stmts)
            s += stmt->ToString() + "\n";
        s += "}";
        return s;
    }

    bool BlockStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            VISIT_AST_HELPER(_vars);
            VISIT_AST_HELPER(_stmts);
        }
        return visitor.EndVisit(*this);
    }

    SymbolType BlockStmtAST::GetSymbolType(const str_t& s, bool recusive) const
    {
        GET_SYMBOLTYPE_HELPER(_vars);
        return DefaultGetSymbolTypeImpl(s, recusive);
    }

    ASTPtr BlockStmtAST::GetSymbol(const str_t& s, bool recusive) const
    {
        GET_SYMBOL_HELPER(_vars);
        return DefaultGetSymbolImpl(s, recusive);
    }

    std::string PrintStmtAST::ToString() const
    {
        std::string s = "print(";
        auto isFirst = true;
        for (const auto& param : _params)
        {
            if (!isFirst)
                s += ", ";
            s += param->ToString();
            isFirst = false;
        }
        s += ");";
        return s;
    }

    bool PrintStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            VISIT_AST_HELPER(_params);
        return visitor.EndVisit(*this);
    }

    std::string ScanStmtAST::ToString() const
    {
        return "scan(" + _name + ");";
    }

    bool ScanStmtAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    std::string AssignStmtAST::ToString() const
    {
        return _name + " = " + _expr->ToString() + ";";
    }

    bool AssignStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            _expr->Accept(visitor);
        return visitor.EndVisit(*this);
    }

    std::string FuncCallStmtAST::ToString() const
    {
        std::string s = _name + "(";
        auto isFirst = true;
        for (const auto& param : _params)
        {
            if (!isFirst)
                s += ", ";
            s += param->ToString();
            isFirst = false;
        }
        s += ");";
        return s;
    }

    bool FuncCallStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            VISIT_AST_HELPER(_params);
        return visitor.EndVisit(*this);
    }

    std::string IfStmtAST::ToString() const
    {
        std::string s = "if (" + _ifcond->ToString() + ")\n" + _ifstmt->ToString();
        if (nullptr != _elsestmt)
            s += "\nelse\n" + _elsestmt->ToString();
        return s;
    }

    bool IfStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            _ifcond->Accept(visitor);
            _ifstmt->Accept(visitor);
            if (nullptr != _elsestmt)
                _elsestmt->Accept(visitor);
        }
        return visitor.EndVisit(*this);
    }

    std::string SwitchStmtAST::ToString() const
    {
        std::string s = "switch (" + _expr->ToString() + ")\n{";
        for (const auto& cs : _stmts)
        {
            if (cs->GetASTType() == ASTType::LabeledStmt)
                s += cs->ToString() + "\n";
            else
                s += "default: \n" + cs->ToString() + "\n";
        }
        s += "}";
        return s;
    }

    bool SwitchStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            VISIT_AST_HELPER(_stmts);
        return visitor.EndVisit(*this);
    }

    std::string LabeledStmtAST::ToString() const
    {
        return "cast " + std::to_string(_int) + ":\n" + _stmt->ToString();
    }

    bool LabeledStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            _stmt->Accept(visitor);
        return visitor.EndVisit(*this);
    }

    std::string WhileStmtAST::ToString() const
    {
        return "while (" + _cond->ToString() + ")\n" + _stmt->ToString();
    }

    bool WhileStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            _cond->Accept(visitor);
            _stmt->Accept(visitor);
        }
        return visitor.EndVisit(*this);
    }

    std::string DoStmtAST::ToString() const
    {
        return "do\n" + _stmt->ToString() + "\n" + "while (" + _cond->ToString() + ")";
    }

    bool DoStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            _stmt->Accept(visitor);
            _cond->Accept(visitor);
        }
        return visitor.EndVisit(*this);
    }

    std::string ForStmtAST::ToString() const
    {
        std::string s = "for (";
        {
            auto isFirst = true;
            for (const auto& expr : _initExprs)
            {
                if (!isFirst)
                    s += ",";
                s += expr->ToString();
                isFirst = false;
            }
        }
        s += "; " + _cond->ToString() + ";";
        {
            auto isFirst = true;
            for (const auto& expr : _updateExprs)
            {
                if (!isFirst)
                    s += ",";
                s += expr->ToString();
                isFirst = false;
            }
        }
        s += ")\n";
        return s + _body->ToString();
    }

    bool ForStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            VISIT_AST_HELPER(_initExprs);
            _cond->Accept(visitor);
            VISIT_AST_HELPER(_updateExprs);
            _body->Accept(visitor);
        }
        return visitor.EndVisit(*this);
    }

    std::string BreakStmtAST::ToString() const
    {
        return "break;";
    }

    bool BreakStmtAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    std::string ContinueStmtAST::ToString() const
    {
        return "continue;";
    }

    bool ContinueStmtAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    std::string ReturnStmtAST::ToString() const
    {
        if (nullptr != _expr)
            return "return " + _expr->ToString() + ";";
        return "return;";
    }

    bool ReturnStmtAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            _expr->Accept(visitor);
        return visitor.EndVisit(*this);
    }
}
