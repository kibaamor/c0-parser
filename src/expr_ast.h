#pragma once
#include "ast.h"

namespace c0
{
    class ExprAST : public AST
    {
    public:
        ExprAST(ASTPtr parent, ASTType type) : AST(parent, type) {}

        virtual VarType GetVarType() const = 0;

        virtual bool IsConst() const { return false; }
        virtual int_t GetInt() const { return 0; }
        virtual char_t GetChar() const { return 0; }
        virtual float_t GetFloat() const { return 0.0; }
    };

    class BinaryExprAST : public ExprAST
    {
    public:
        BinaryExprAST(ASTPtr parent, ExprASTPtr left, BinaryType ot, ExprASTPtr right)
            : ExprAST(parent, ASTType::BinaryExpr)
            , _left(left)
            , _ot(ot)
            , _right(right)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        bool IsCond() const { return _ot >= BinaryType::Less && _ot <= BinaryType::GreaterEqual; }
        const ExprASTPtr& GetLeftExpr() const { return _left; }
        BinaryType GetOT() const { return _ot; }
        const ExprASTPtr& GetRightExpr() const { return _right; }

    private:
        ExprASTPtr _left;
        BinaryType _ot;
        ExprASTPtr _right;
    };

    class CastExprAST : public ExprAST
    {
    public:
        CastExprAST(ASTPtr parent, ExprASTPtr expr, VarType type, bool isExplicit)
            : ExprAST(parent, ASTType::CastExpr)
            , _expr(expr)
            , _type(type)
            , _isExplicit(isExplicit)
        {
        }

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        const ExprASTPtr& GetExpr() const { return _expr; }
        bool IsExplicit() const { return _isExplicit; }

    private:
        ExprASTPtr _expr;
        VarType _type;
        bool _isExplicit;
    };

    class UnaryExprAST : public ExprAST
    {
    public:
        UnaryExprAST(ASTPtr parent, UnaryType ut, ExprASTPtr expr) 
            : ExprAST(parent, ASTType::UnaryExpr)
            , _ut(ut)
            , _expr(expr)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        bool IsConst() const override;
        int_t GetInt() const override;
        char_t GetChar() const override;
        float_t GetFloat() const override;

        UnaryType GetUT() const { return _ut; }
        const ExprASTPtr& GetExpr() const { return _expr; }

    private:
        UnaryType _ut;
        ExprASTPtr _expr;
    };

    class PrimaryExprAST : public ExprAST
    {
    public:
        PrimaryExprAST(ASTPtr parent, ASTType type) : ExprAST(parent, type) {}
    };

    class BraceExprAST : public PrimaryExprAST
    {
    public:
        BraceExprAST(ASTPtr parent, ExprASTPtr expr)
            : PrimaryExprAST(parent, ASTType::BraceExpr)
            , _expr(expr)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        const ExprASTPtr& GetExpr() const { return _expr; }

    private:
        ExprASTPtr _expr;
    };

    class IdentExprAST : public PrimaryExprAST
    {
    public:
        IdentExprAST(ASTPtr parent, str_t name) : PrimaryExprAST(parent, ASTType::IdentExpr), _name(name) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        const str_t& GetName() const { return _name; }

    private:
        str_t _name;
    };

    class IntExprAST : public PrimaryExprAST
    {
    public:
        IntExprAST(ASTPtr parent, int_t i) : PrimaryExprAST(parent, ASTType::IntExpr), _int(i) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        bool IsConst() const override { return true; }
        int_t GetInt() const override { return _int; }

    private:
        int_t _int;
    };

    class CharExprAST : public PrimaryExprAST
    {
    public:
        CharExprAST(ASTPtr parent, char_t c) : PrimaryExprAST(parent, ASTType::CharExpr), _char(c) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        bool IsConst() const override { return true; }
        char_t GetChar() const override { return _char; }

    private:
        char_t _char;
    };

    class FloatExprAST : public PrimaryExprAST
    {
    public:
        FloatExprAST(ASTPtr parent, float_t f) : PrimaryExprAST(parent, ASTType::FloatExpr), _float(f) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        bool IsConst() const override { return true; }
        float_t GetFloat() const override { return _float; }

    private:
        float_t _float;
    };

    class StrExprAST : public PrimaryExprAST
    {
    public:
        StrExprAST(ASTPtr parent, const str_t& str) : PrimaryExprAST(parent, ASTType::StrExpr), _str(str) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        const str_t& GetStr() const { return _str; }

    private:
        str_t _str;
    };

    class AssignExprAST : public PrimaryExprAST
    {
    public:
        AssignExprAST(ASTPtr parent, str_t name, ExprASTPtr expr)
            : PrimaryExprAST(parent, ASTType::AssignExpr)
            , _name(name)
            , _expr(expr)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        const str_t& GetName() const { return _name; }
        const ExprASTPtr& GetExpr() const { return _expr; }

    private:
        str_t _name;
        ExprASTPtr _expr;
    };

    class FuncCallExprAST : public PrimaryExprAST
    {
    public:
        FuncCallExprAST(ASTPtr parent, str_t name) : PrimaryExprAST(parent, ASTType::FuncCallExpr), _name(name) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        VarType GetVarType() const override;

        void AddParam(ExprASTPtr ptr) { _params.push_back(ptr); }

        const str_t& GetName() const { return _name; }
        const ExprASTPtrList& GetParams() const { return _params; }

    private:
        str_t _name;
        ExprASTPtrList _params;
    };

}
