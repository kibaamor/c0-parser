#pragma once
#include "ast.h"

namespace c0
{
    class StmtAST : public AST
    {
    public:
        StmtAST(ASTPtr parent, ASTType type) : AST(parent, type) {}
    };

    class EmptyStmtAST : public StmtAST
    {
    public:
        EmptyStmtAST(ASTPtr parent) : StmtAST(parent, ASTType::EmptyStmt) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        SymbolType GetSymbolType(const str_t& s, bool recusive) const override;
        ASTPtr GetSymbol(const str_t& s, bool recusive) const override;
    };

    class BlockStmtAST : public StmtAST
    {
    public:
        BlockStmtAST(ASTPtr parent) : StmtAST(parent, ASTType::BlockStmt) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        SymbolType GetSymbolType(const str_t& s, bool recusive) const override;
        ASTPtr GetSymbol(const str_t& s, bool recusive) const override;

        void AddVar(VarDeclASTPtr ptr) { _vars.push_back(ptr); }
        void AddStmt(StmtASTPtr ptr) { _stmts.push_back(ptr); }

        const VarDeclASTPtrList& GetVars() const { return _vars; }
        const StmtASTPtrList& GetStmts() const { return _stmts; }

    private:
        VarDeclASTPtrList _vars;
        StmtASTPtrList _stmts;
    };

    class CondStmtAST : public StmtAST
    {
    public:
        CondStmtAST(ASTPtr parent, ASTType type) : StmtAST(parent, type) {}
    };

    class LoopStmtAST : public StmtAST
    {
    public:
        LoopStmtAST(ASTPtr parent, ASTType type) : StmtAST(parent, type) {}
    };

    class JumpStmtAST : public StmtAST
    {
    public:
        JumpStmtAST(ASTPtr parent, ASTType type) : StmtAST(parent, type) {}
    };

    class PrintStmtAST : public StmtAST
    {
    public:
        PrintStmtAST(ASTPtr parent) : StmtAST(parent, ASTType::PrintStmt) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        void AddParam(ExprASTPtr ptr) { _params.push_back(ptr); }
        const ExprASTPtrList& GetParams() const { return _params; }

    private:
        ExprASTPtrList _params;
    };

    class ScanStmtAST : public StmtAST
    {
    public:
        ScanStmtAST(ASTPtr parent, str_t name) : StmtAST(parent, ASTType::ScanStmt), _name(name) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        const str_t& GetName() const { return _name; }

    private:
        const str_t _name;
    };

    class AssignStmtAST : public StmtAST
    {
    public:
        AssignStmtAST(ASTPtr parent, str_t name, ExprASTPtr expr) 
            : StmtAST(parent, ASTType::AssignStmt)
            , _name(name)
            , _expr(expr)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        const str_t& GetName() const { return _name; }
        const ExprASTPtr& GetExpr() const { return _expr; }

    private:
        const str_t _name;
        const ExprASTPtr _expr;
    };

    class FuncCallStmtAST : public StmtAST
    {
    public:
        FuncCallStmtAST(ASTPtr parent, str_t name) : StmtAST(parent, ASTType::FuncCallStmt), _name(name) {}

        void AddParam(ExprASTPtr ptr) { _params.push_back(ptr); }

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        const str_t& GetName() const { return _name; }
        const ExprASTPtrList& GetParams() const { return _params; }

    private:
        const str_t _name;
        ExprASTPtrList _params;
    };

    //-------------------------------------------------------------------------

    class IfStmtAST : public CondStmtAST
    {
    public:
        IfStmtAST(ASTPtr parent, BinaryExprASTPtr ifcond, StmtASTPtr ifstmt)
            : CondStmtAST(parent, ASTType::IfStmt)
            , _ifcond(ifcond)
            , _ifstmt(ifstmt)
        {}

        void SetElseStmt(StmtASTPtr ptr) { _elsestmt = ptr; }

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        const BinaryExprASTPtr& GetIfCond() const { return _ifcond; }
        const StmtASTPtr& GetIFStmt() const { return _ifstmt; }
        const StmtASTPtr& GetElseStmt() const { return _elsestmt; }

    private:
        const BinaryExprASTPtr _ifcond;
        const StmtASTPtr _ifstmt;
        StmtASTPtr _elsestmt;
    };

    class SwitchStmtAST : public CondStmtAST
    {
    public:
        SwitchStmtAST(ASTPtr parent, ExprASTPtr expr)
            : CondStmtAST(parent, ASTType::SwitchStmt)
            , _expr(expr)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        const ExprASTPtr& GetExpr() const { return _expr; }
        const StmtASTPtrList& GetStmts() const { return _stmts; }

        void AddStmt(StmtASTPtr ptr) { _stmts.push_back(ptr); }

    private:
        const ExprASTPtr _expr;
        StmtASTPtrList _stmts;
    };

    class LabeledStmtAST : public CondStmtAST
    {
    public:
        LabeledStmtAST(ASTPtr parent, int_t i, StmtASTPtr stmt)
            : CondStmtAST(parent, ASTType::LabeledStmt)
            , _int(i)
            , _stmt(stmt)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        int_t GetInt() const { return _int; }
        const StmtASTPtr& GetStmt() const { return _stmt; }

    private:
        const int_t _int;
        const StmtASTPtr _stmt;
    };

    class WhileStmtAST : public LoopStmtAST
    {
    public:
        WhileStmtAST(ASTPtr parent, BinaryExprASTPtr cond, StmtASTPtr stmt)
            : LoopStmtAST(parent, ASTType::WhileStmt)
            , _cond(cond)
            , _stmt(stmt)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        const BinaryExprASTPtr& GetCond() const { return _cond; }
        const StmtASTPtr& GetStmt() const { return _stmt; }

    private:
		const BinaryExprASTPtr _cond;
        const StmtASTPtr _stmt;
    };


    class DoStmtAST : public LoopStmtAST
    {
    public:
        DoStmtAST(ASTPtr parent, StmtASTPtr stmt, BinaryExprASTPtr cond)
            : LoopStmtAST(parent, ASTType::DoStmt)
            , _stmt(stmt)
            , _cond(cond)
        {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        const StmtASTPtr& GetStmt() const { return _stmt; }
        const BinaryExprASTPtr& GetCond() const { return _cond; }

    private:
        const StmtASTPtr _stmt;
        const BinaryExprASTPtr _cond;
    };
    
    class ForStmtAST : public LoopStmtAST
    {
    public:
        ForStmtAST(ASTPtr parent) : LoopStmtAST(parent, ASTType::ForStmt) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        void AddInitExpr(AssignExprASTPtr ptr) { _initExprs.push_back(ptr); }
        void SetCond(BinaryExprASTPtr ptr) { _cond = ptr; }
        void AddUpdateExpr(ExprASTPtr ptr) { _updateExprs.push_back(ptr); }
        void SetBody(StmtASTPtr ptr) { _body = ptr; }

        const AssignExprASTPtrList& GetInitExprs() const { return _initExprs; }
        const BinaryExprASTPtr& GetCond() const { return _cond; }
        const ExprASTPtrList& GetUpdateExprs() const { return _updateExprs; }
        const StmtASTPtr& GetBody() const { return _body; }

    private:
        AssignExprASTPtrList _initExprs;
        BinaryExprASTPtr _cond;
        ExprASTPtrList _updateExprs;

        StmtASTPtr _body;
    };

    class BreakStmtAST : public JumpStmtAST
    {
    public:
        BreakStmtAST(ASTPtr parent) : JumpStmtAST(parent, ASTType::BreakStmt) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;
    };

    class ContinueStmtAST : public JumpStmtAST
    {
    public:
        ContinueStmtAST(ASTPtr parent) : JumpStmtAST(parent, ASTType::ContinueStmt) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;
    };

    class ReturnStmtAST : public JumpStmtAST
    {
    public:
        ReturnStmtAST(ASTPtr parent) : JumpStmtAST(parent, ASTType::ReturnStmt) {}

        void SetExpr(ExprASTPtr ptr) { _expr = ptr; }

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;

        const ExprASTPtr& GetExpr() const { return _expr; }

    private:
        ExprASTPtr _expr;
    };
}
