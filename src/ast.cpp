#include "all_ast.h"
#include <atomic>

namespace c0
{
    std::atomic<std::int32_t> _astInstanceCount = {0};
    instancemap_t _astInstanceMap;

    int32_t AST::GetInstanceCount()
    {
        return _astInstanceCount.load(std::memory_order::memory_order_acquire);
    }

    const instancemap_t& AST::GetInstanceMap()
    {
        return _astInstanceMap;
    }

    AST::AST(ASTPtr parent, ASTType type)
        : _parent(parent)
        , _type(type)
    {
        ++_astInstanceMap[_type];
        _astInstanceCount.fetch_add(1, std::memory_order::memory_order_release);
    }

    AST::~AST()
    {
        _astInstanceCount.fetch_sub(1, std::memory_order::memory_order_release);
        --_astInstanceMap[_type];
    }

    bool FileAST::Accept(ASTVisitor& visitor) const
    {
        if (visitor.BegVisit(*this))
        {
            VISIT_AST_HELPER(_vars);
            VISIT_AST_HELPER(_funcs);
        }
        return visitor.EndVisit(*this);
    }

    std::string FileAST::ToString() const
    {
        std::string s;
        for (const auto& var : _vars)
            s += var->ToString() + ";\n";
        if (!_vars.empty())
            s += "\n";
        for (const auto& func : _funcs)
            s += func->ToString() + "\n\n";

        return s;
    }

    SymbolType FileAST::GetSymbolType(const str_t& s, bool recusive) const
    {
        GET_SYMBOLTYPE_HELPER(_vars);
        for (const auto& func : _funcs)
        {
            if (s == func->GetName())
                return SymbolType::Func;
        }
        return DefaultGetSymbolTypeImpl(s, recusive);
    }

    ASTPtr FileAST::GetSymbol(const str_t& s, bool recusive) const
    {
        GET_SYMBOL_HELPER(_vars);
        for (const auto& func : _funcs)
        {
            if (s == func->GetName())
                return func;
        }
        return DefaultGetSymbolImpl(s, recusive);
    }

    VarType TokenType2VarType(TokenType type)
    {
        switch (type)
        {
        case TokenType::R_VOID: return VarType::Void;
        case TokenType::R_INT: return VarType::Int;
        case TokenType::R_CHAR: return VarType::Char;
        case TokenType::R_DOUBLE: return VarType::Float;
        }
        return VarType::Nul;
    }

    VarType MergeVarType(VarType a, VarType b)
    {
        if (VarType::Float == a || VarType::Float == b)
            return VarType::Float;
        if (VarType::Void == a || VarType::Void == b)
            return a == b ? VarType::Void : VarType::Nul;
        return VarType::Int;
    }

    bool IsValidCastType(VarType t)
    {
        return t >= VarType::Int && t <= VarType::Float;
    }

    bool IsVarTypeCastable(VarType from, VarType to)
    {
        return from >= VarType::Int && from <= VarType::Float
            && to >= VarType::Int && to <= VarType::Float;
    }
}

namespace std
{
    using namespace c0;
    string to_string(BinaryType ot)
    {
        switch (ot)
        {
        case BinaryType::Add:           return "+";
        case BinaryType::Sub:           return "-";
        case BinaryType::Mul:           return "*";
        case BinaryType::Div:           return "/";
        case BinaryType::Less:          return "<";
        case BinaryType::LessEqual:     return "<=";
        case BinaryType::Euqal:         return "==";
        case BinaryType::NotEqual:      return "!=";
        case BinaryType::Greater:       return ">";
        case BinaryType::GreaterEqual:  return ">=";
        }
        return "Nul";
    }

    string to_string(c0::UnaryType ut)
    {
        switch (ut)
        {
        case UnaryType::Positive: return "+";
        case UnaryType::Negative: return "-";
        }
        return "Nul";
    }

    string to_string(VarType vt)
    {
        switch (vt)
        {
        case VarType::Void:  return "void";
        case VarType::Int:   return "int";
        case VarType::Char:  return "char";
        case VarType::Float: return "double";
        case VarType::Str:   return "string";
        }
        return "Nul";
    }

    string to_string(c0::ASTPtr ptr)
    {
        if (nullptr != ptr)
            return ptr->ToString();
        return "nullptr";
    }

    string to_string(c0::ASTType at)
    {
#define ASTTYPE_TOSTRING_HELPER(name) case c0::ASTType::name: return #name
        switch (at)
        {
            ASTTYPE_TOSTRING_HELPER(BinaryExpr);
            ASTTYPE_TOSTRING_HELPER(CastExpr);
            ASTTYPE_TOSTRING_HELPER(UnaryExpr);
            ASTTYPE_TOSTRING_HELPER(BraceExpr);
            ASTTYPE_TOSTRING_HELPER(IdentExpr);
            ASTTYPE_TOSTRING_HELPER(IntExpr);
            ASTTYPE_TOSTRING_HELPER(CharExpr);
            ASTTYPE_TOSTRING_HELPER(FloatExpr);
            ASTTYPE_TOSTRING_HELPER(StrExpr);
            ASTTYPE_TOSTRING_HELPER(AssignExpr);
            ASTTYPE_TOSTRING_HELPER(FuncCallExpr);
            ASTTYPE_TOSTRING_HELPER(EmptyStmt);
            ASTTYPE_TOSTRING_HELPER(BlockStmt);
            ASTTYPE_TOSTRING_HELPER(PrintStmt);
            ASTTYPE_TOSTRING_HELPER(ScanStmt);
            ASTTYPE_TOSTRING_HELPER(AssignStmt);
            ASTTYPE_TOSTRING_HELPER(FuncCallStmt);
            ASTTYPE_TOSTRING_HELPER(IfStmt);
            ASTTYPE_TOSTRING_HELPER(SwitchStmt);
            ASTTYPE_TOSTRING_HELPER(LabeledStmt);
            ASTTYPE_TOSTRING_HELPER(WhileStmt);
            ASTTYPE_TOSTRING_HELPER(DoStmt);
            ASTTYPE_TOSTRING_HELPER(ForStmt);
            ASTTYPE_TOSTRING_HELPER(BreakStmt);
            ASTTYPE_TOSTRING_HELPER(ContinueStmt);
            ASTTYPE_TOSTRING_HELPER(ReturnStmt);
            ASTTYPE_TOSTRING_HELPER(VarDecl);
            ASTTYPE_TOSTRING_HELPER(FuncDecl);
            ASTTYPE_TOSTRING_HELPER(File);
        }
        return "Nul";
    }
}
