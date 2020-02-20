#pragma once
#include "ast.h"

namespace c0
{
    class DeclAST : public AST
    {
    public:
        DeclAST(ASTPtr parent, ASTType type) : AST(parent, type) {}

        virtual DeclType GetDeclType() const = 0;
        virtual VarType GetVarType() const = 0;
    };

    class VarDeclAST : public DeclAST
    {
    public:
        VarDeclAST(ASTPtr parent, bool isParam, bool isConst, VarType vt, const str_t& name)
            : DeclAST(parent, ASTType::VarDecl), _isParam(isParam), _isConst(isConst), _vt(vt), _name(name)
        {}

        DeclType GetDeclType() const override { return _isConst ? DeclType::ConstVar : DeclType::Var; }
        VarType GetVarType() const override { return _vt; }

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;
        SymbolType GetSymbolType(const str_t& s, bool recusive) const override;
        ASTPtr GetSymbol(const str_t& s, bool recusive) const override;

        bool IsParam() const { return _isParam; }
        bool IsConst() const { return _isConst; }
        const str_t& GetName() const { return _name; }
        ExprASTPtr GetExpr() const { return _expr; }
        void SetExpr(ExprASTPtr ptr) { _expr = ptr; }
        bool HasExpr() const { return nullptr != _expr; }

    private:
        const bool _isParam;
        const bool _isConst;
        VarType _vt;
        str_t _name;
        ExprASTPtr _expr;
    };

    class FuncDeclAST : public DeclAST
    {
    public:
        FuncDeclAST(ASTPtr parent, VarType retType, const str_t& name) 
            : DeclAST(parent, ASTType::FuncDecl), _retType(retType), _name(name)
        {}

        DeclType GetDeclType() const override { return DeclType::Func; }
        VarType GetVarType() const override { return _retType; }

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;
        SymbolType GetSymbolType(const str_t& s, bool recusive) const override;
        ASTPtr GetSymbol(const str_t& s, bool recusive) const override;

        void AddParam(VarDeclASTPtr ptr) { _params.push_back(ptr); }
        void SetBlockStmt(BlockStmtASTPtr ptr) { _block = ptr; }

        const str_t& GetName() const { return _name; }
        const VarDeclASTPtrList& GetParams() const { return _params; }
        BlockStmtASTPtr GetBlockStmt() const { return _block; }

    private:
        VarType _retType;
        str_t _name;
        VarDeclASTPtrList _params;
        BlockStmtASTPtr _block;
    };
}
