#pragma once
#include <memory>
#include <vector>
#include <map>
#include "token.h"

namespace c0
{
    enum class SymbolType
    {
        Nul,
        Var,
        ConstVar,
        Func,
    };

    enum class BinaryType
    {
        Nul,
        Add,            // +
        Sub,            // -
        Mul,            // *
        Div,            // /
        Less,           // <
        LessEqual,      // <=
        Euqal,          // ==
        NotEqual,       // !=
        Greater,        // >
        GreaterEqual,   // >=
    };

    enum class UnaryType
    {
        Nul,
        Positive,
        Negative,
    };

    enum class DeclType
    {
        Nul,
        Var,
        ConstVar,
        Func,
    };

    enum class VarType
    {
        Nul,
        Void,
        Int,
        Char,
        Float,
        Str,
    };

    enum class ASTType
    {
        Nul,

        BinaryExpr,
        CastExpr,
        UnaryExpr,

        BraceExpr,
        IdentExpr,
        IntExpr,
        CharExpr,
        FloatExpr,
        StrExpr,

        AssignExpr,
        FuncCallExpr,

        EmptyStmt,
        BlockStmt,
        PrintStmt,
        ScanStmt,
        AssignStmt,
        FuncCallStmt,

        IfStmt,
        SwitchStmt,
        LabeledStmt,

        WhileStmt,
        DoStmt,
        ForStmt,

        BreakStmt,
        ContinueStmt,
        ReturnStmt,

        VarDecl,
        FuncDecl,

        File,
    };

#define AST_DECL_HELPER(name) \
    class name; \
    using name##Ptr = std::shared_ptr<name>; \
    using name##PtrList = std::vector<name##Ptr>

    AST_DECL_HELPER(AST);

    AST_DECL_HELPER(ExprAST);
    AST_DECL_HELPER(BinaryExprAST);
    AST_DECL_HELPER(CastExprAST);
    AST_DECL_HELPER(UnaryExprAST);
    AST_DECL_HELPER(PrimaryExprAST);

    AST_DECL_HELPER(BraceExprAST);
    AST_DECL_HELPER(IdentExprAST);
    AST_DECL_HELPER(IntExprAST);
    AST_DECL_HELPER(CharExprAST);
    AST_DECL_HELPER(FloatExprAST);
    AST_DECL_HELPER(StrExprAST);

    AST_DECL_HELPER(AssignExprAST);
    AST_DECL_HELPER(FuncCallExprAST);

    AST_DECL_HELPER(StmtAST);
    AST_DECL_HELPER(EmptyStmtAST);
    AST_DECL_HELPER(BlockStmtAST);
    AST_DECL_HELPER(CondStmtAST);
    AST_DECL_HELPER(LoopStmtAST);
    AST_DECL_HELPER(JumpStmtAST);
    AST_DECL_HELPER(PrintStmtAST);
    AST_DECL_HELPER(ScanStmtAST);
    AST_DECL_HELPER(AssignStmtAST);
    AST_DECL_HELPER(FuncCallStmtAST);

    AST_DECL_HELPER(IfStmtAST);
    AST_DECL_HELPER(SwitchStmtAST);
    AST_DECL_HELPER(LabeledStmtAST);

    AST_DECL_HELPER(WhileStmtAST);
    AST_DECL_HELPER(DoStmtAST);
    AST_DECL_HELPER(ForStmtAST);

    AST_DECL_HELPER(BreakStmtAST);
    AST_DECL_HELPER(ContinueStmtAST);
    AST_DECL_HELPER(ReturnStmtAST);

    AST_DECL_HELPER(DeclAST);
    AST_DECL_HELPER(VarDeclAST);
    AST_DECL_HELPER(FuncDeclAST);

    AST_DECL_HELPER(FileAST);

#undef AST_DECL_HELPER

#define VISIT_AST_HELPER(name)                      \
    for (const auto& ptr : name)                    \
    {                                               \
        if (!ptr->Accept(visitor))                  \
            break;                                  \
    }

#define GET_SYMBOLTYPE_HELPER(name)                 \
    for (const auto& ptr : name)                    \
    {                                               \
        const auto t = ptr->GetSymbolType(s, false);\
        if (SymbolType::Nul != t)                   \
            return t;                               \
    }

#define GET_SYMBOL_HELPER(name)                     \
    for (const auto& ptr : name)                    \
    {                                               \
        const auto p = ptr->GetSymbol(s, false);    \
        if (nullptr != p)                           \
            return p;                               \
    }

    class ASTVisitor
    {
    public:
        virtual ~ASTVisitor() = default;

        virtual bool BegVisit(const AST& ast) = 0;
        virtual bool EndVisit(const AST& ast) = 0;
    };

    class ASTUserData
    {
    public:
        virtual ~ASTUserData() = default;
    };
    using ASTUserDataPtr = std::shared_ptr<ASTUserData>;

    using instancemap_t = std::map<ASTType, std::int32_t>;

    class AST : public std::enable_shared_from_this<AST>
    {
    public:
        static int32_t GetInstanceCount();
        static const instancemap_t& GetInstanceMap();

    public:
        AST(ASTPtr parent, ASTType type);
        virtual ~AST();

        ASTType GetASTType() const { return _type; }
        virtual std::string ToString() const = 0;
        virtual bool Accept(ASTVisitor& visitor) const = 0;

        virtual SymbolType GetSymbolType(const str_t& s, bool recusive) const
        {
            return DefaultGetSymbolTypeImpl(s, recusive);
        }
        virtual ASTPtr GetSymbol(const str_t& s, bool recusive) const
        {
            return DefaultGetSymbolImpl(s, recusive);
        }

        ASTPtr GetParent() const { return _parent.lock(); }
        void SetParent(ASTPtr parent) { _parent = parent; }
        const ASTUserDataPtr& GetUserData() const { return _userdata; }
        void SetUserData(ASTUserDataPtr ptr) const { _userdata = ptr; }

    protected:
        SymbolType DefaultGetSymbolTypeImpl(const str_t& s, bool recusive) const
        {
            if (recusive)
            {
                auto parent = GetParent();
                if (nullptr != parent)
                    return parent->GetSymbolType(s, recusive);
            }
            return SymbolType::Nul;
        }
        ASTPtr DefaultGetSymbolImpl(const str_t& s, bool recusive) const
        {
            if (recusive)
            {
                auto parent = GetParent();
                if (nullptr != parent)
                    return parent->GetSymbol(s, recusive);
            }
            return nullptr;
        }

    private:
        std::weak_ptr<AST> _parent;
        ASTType _type;
        mutable ASTUserDataPtr _userdata;
    };

    class FileAST : public AST
    {
    public:
        FileAST(ASTPtr parent) : AST(parent, ASTType::File) {}

        std::string ToString() const override;
        bool Accept(ASTVisitor& visitor) const override;
        SymbolType GetSymbolType(const str_t& s, bool recusive) const override;
        ASTPtr GetSymbol(const str_t& s, bool recusive) const override;

        void AddVar(VarDeclASTPtr ptr) { _vars.push_back(ptr); }
        void AddFunc(FuncDeclASTPtr ptr) { _funcs.push_back(ptr); }

        const VarDeclASTPtrList& GetVars() const { return _vars; }
        const FuncDeclASTPtrList GetFuncs() const { return _funcs; }

    private:
        VarDeclASTPtrList _vars;
        FuncDeclASTPtrList _funcs;
    };

    VarType TokenType2VarType(TokenType type);
    VarType MergeVarType(VarType a, VarType b);
    bool IsValidCastType(VarType t);
    bool IsVarTypeCastable(VarType from, VarType to);
}

namespace std
{
    string to_string(c0::BinaryType ot);
    string to_string(c0::UnaryType ut);
    string to_string(c0::VarType vt);
    string to_string(c0::ASTPtr ptr);
    string to_string(c0::ASTType at);
}
