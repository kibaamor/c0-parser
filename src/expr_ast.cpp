#include "all_ast.h"

namespace c0
{
    std::string BinaryExprAST::ToString() const
    {
        return _left->ToString() + " " + std::to_string(_ot) + " " + _right->ToString();
    }

    bool BinaryExprAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            _left->Accept(visitor);
            if (nullptr != _right)
                _right->Accept(visitor);
        }
        return visitor.EndVisit(*this);
    }

    VarType BinaryExprAST::GetVarType() const
    {
        const auto leftVT = _left->GetVarType();
        const auto rightVT = _right->GetVarType();
        return MergeVarType(leftVT, rightVT);
    }

    std::string CastExprAST::ToString() const
    {
        return
#if 0
            std::string(_isExplicit ? "" : "/*inexplicit cast*/") + 
#endif
            "(" + std::to_string(_type) + ")(" + _expr->ToString() + ")";
    }

    bool CastExprAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            _expr->Accept(visitor);
        return visitor.EndVisit(*this);
    }

    VarType CastExprAST::GetVarType() const
    {
        return _type;
    }

    std::string UnaryExprAST::ToString() const
    {
        return std::to_string(_ut) + _expr->ToString();
    }

    bool UnaryExprAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            _expr->Accept(visitor);
        return visitor.EndVisit(*this);
    }

    VarType UnaryExprAST::GetVarType() const
    {
        return _expr->GetVarType();
    }

    bool UnaryExprAST::IsConst() const
    {
        return _expr->IsConst();
    }

    int_t UnaryExprAST::GetInt() const
    {
        const auto v = _expr->GetInt();
        return UnaryType::Negative == _ut ? (-v) : v;
    }

    char_t UnaryExprAST::GetChar() const
    {
        const auto v = _expr->GetChar();
        return UnaryType::Negative == _ut ? (-v) : v;
    }

    float_t UnaryExprAST::GetFloat() const
    {
        const auto v = _expr->GetFloat();
        return UnaryType::Negative == _ut ? (-v) : v;
    }

    std::string BraceExprAST::ToString() const
    {
        return std::string("(") + _expr->ToString() + ")";
    }

    bool BraceExprAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            _expr->Accept(visitor);
        return visitor.EndVisit(*this);
    }

    VarType BraceExprAST::GetVarType() const
    {
        return _expr->GetVarType();
    }

    std::string IdentExprAST::ToString() const
    {
        return _name;
    }

    bool IdentExprAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    VarType IdentExprAST::GetVarType() const
    {
        const auto decl = std::dynamic_pointer_cast<DeclAST>(GetSymbol(_name, true));
        if (nullptr != decl)
            return decl->GetVarType();
        return VarType::Nul;
    }

    std::string IntExprAST::ToString() const
    {
        return std::to_string(_int);
    }

    bool IntExprAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    VarType IntExprAST::GetVarType() const
    {
        return VarType::Int;
    }

    std::string CharExprAST::ToString() const
    {
        return Token(_char, posrange_t()).GetValueString();
    }

    bool CharExprAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    VarType CharExprAST::GetVarType() const
    {
        return VarType::Char;
    }

    std::string FloatExprAST::ToString() const
    {
        return std::to_string(_float);
    }

    bool FloatExprAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    VarType FloatExprAST::GetVarType() const
    {
        return VarType::Float;
    }

    std::string StrExprAST::ToString() const
    {
        return "\"" + _str + "\"";
    }

    bool StrExprAST::Accept(ASTVisitor& visitor) const
    {
        visitor.BegVisit(*this);
        return visitor.EndVisit(*this);
    }

    VarType StrExprAST::GetVarType() const
    {
        return VarType::Str;
    }

    std::string AssignExprAST::ToString() const
    {
        return _name + " = " + _expr->ToString();
    }

    bool AssignExprAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            _expr->Accept(visitor);
        return visitor.EndVisit(*this);
    }

    VarType AssignExprAST::GetVarType() const
    {
        const auto decl = std::dynamic_pointer_cast<DeclAST>(GetSymbol(_name, true));
        if (nullptr != decl)
            return decl->GetVarType();
        return VarType::Nul;
    }

    std::string FuncCallExprAST::ToString() const
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
        s += ")";
        return s;
    }

    bool FuncCallExprAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
            VISIT_AST_HELPER(_params);
        return visitor.EndVisit(*this);
    }

    VarType FuncCallExprAST::GetVarType() const
    {
        const auto decl = std::dynamic_pointer_cast<DeclAST>(GetSymbol(_name, true));
        if (nullptr != decl)
            return decl->GetVarType();
        return VarType::Nul;
    }
}
