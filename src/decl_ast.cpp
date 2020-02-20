#include "all_ast.h"

namespace c0
{
    std::string VarDeclAST::ToString() const
    {
        std::string s;
        if (_isConst)
            s = "const ";
        s += std::to_string(GetVarType()) + " " + _name;
        if (nullptr != _expr)
            s += " = " + _expr->ToString();
        return s;
    }

    bool VarDeclAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            if (nullptr != _expr)
                return _expr->Accept(visitor);
        }
        return visitor.EndVisit(*this);
    }

    SymbolType VarDeclAST::GetSymbolType(const str_t& s, bool recusive) const
    {
        if (s == _name)
            return _isConst ? SymbolType::ConstVar : SymbolType::Var;
        return DefaultGetSymbolTypeImpl(s, recusive);
    }

    ASTPtr VarDeclAST::GetSymbol(const str_t& s, bool recusive) const
    {
        if (s == _name)
            return const_cast<VarDeclAST* const>(this)->shared_from_this();
        return DefaultGetSymbolImpl(s, recusive);
    }

    std::string FuncDeclAST::ToString() const
    {
        std::string s = std::to_string(_retType) + " " + _name + "(";
        auto firstParam = true;
        for (const auto& param : _params)
        {
            if (!firstParam)
                s += ", ";
            s += param->ToString();
            firstParam = false;
        }
        s += ")\n";
        if (nullptr != _block)
            s += _block->ToString();
        return s;
    }

    bool FuncDeclAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            VISIT_AST_HELPER(_params);
            _block->Accept(visitor);
        }
        return visitor.EndVisit(*this);
    }

    SymbolType FuncDeclAST::GetSymbolType(const str_t& s, bool recusive) const
    {
        if (s == _name)
            return SymbolType::Func;
        GET_SYMBOLTYPE_HELPER(_params);
        return DefaultGetSymbolTypeImpl(s, recusive);
    }

    ASTPtr FuncDeclAST::GetSymbol(const str_t& s, bool recusive) const
    {
        if (s == _name)
            return const_cast<FuncDeclAST* const>(this)->shared_from_this();
        GET_SYMBOL_HELPER(_params);
        return DefaultGetSymbolImpl(s, recusive);
    }
}