#include "analyser.h"

namespace c0
{
    /*
    <variable-declaration> ::=
        [<const-qualifier>]<type-specifier><init-declarator-list>';'
    <init-declarator-list> ::=
        <init-declarator>{','<init-declarator>}
    <init-declarator> ::=
        <identifier>[<initializer>]
    <initializer> ::=
        '='<expression>
    */
    VarDeclASTPtrList Analyser::AnalyseVarDecl(ASTPtr parent, AnalyseError& err)
    {
        VarDeclASTPtrList varlist;

        bool isConst = false;
        auto token = ReadToken();
        if (token.GetType() == TokenType::R_CONST)
        {
            isConst = true;
            token = ReadToken();
        }

        const auto varType = TokenType2VarType(token.GetType());
        if (VarType::Nul == varType)
        {
            err = AnalyseError("expect type-specifier", token);
            return varlist;
        }

        while (true)
        {
            token = ReadToken();
            if (token.GetType() != TokenType::IDENT)
            {
                err = AnalyseError("expect variable name", token);
                return varlist;
            }
            const auto varName = token.GetString();
            const auto t = parent->GetSymbolType(varName, false);
            if (t != SymbolType::Nul)
            {
                err = AnalyseError("variable name repeated", token);
                return varlist;
            }

            auto var = std::make_shared<VarDeclAST>(parent, false, isConst, varType, varName);
            varlist.push_back(var);

            token = ReadToken();

            if (token.GetType() == TokenType::S_ASSIGN)
            {
                auto expr = AnalyseExpr(parent, err, isConst);
                if (err)
                    return varlist;
                expr = CheckInexplicitTypeCast(parent, err, token, expr, varType, 
                    "invalid variable declare, ");
                if (err)
                    return varlist;
                var->SetExpr(expr);
                token = ReadToken();
            }
            
            if (token.GetType() == TokenType::S_SEMICOLON)
            {
                break;
            }
            else if (token.GetType() == TokenType::S_COMMA)
            {
            }
            else
            {
                err = AnalyseError("invalid variable declare", token);
                return varlist;
            }
        }

        return varlist;
    }

    /*
    <function-definition> ::=
        <type-specifier><identifier><parameter-clause><compound-statement>
        
    <parameter-clause> ::=
        '(' [<parameter-declaration-list>] ')'
    <parameter-declaration-list> ::=
        <parameter-declaration>{','<parameter-declaration>}
    */
    FuncDeclASTPtr Analyser::AnalyseFuncDecl(ASTPtr parent, AnalyseError& err)
    {
        auto token = ReadToken();
        const auto retType = TokenType2VarType(token.GetType());
        if (VarType::Nul == retType)
        {
            err = AnalyseError("expect type-specifier for function return type", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::IDENT)
        {
            err = AnalyseError("expect function name", token);
            return nullptr;
        }
        const auto funcName = token.GetString();

        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' after function name", token);
            return nullptr;
        }

        auto func = std::make_shared<FuncDeclAST>(parent, retType, funcName);

        for (token = PeekToken(); 
            token.GetType() != TokenType::S_RBRACES; 
            token = PeekToken())
        {
            auto param = AnalyseFuncDeclParam(func, err);
            if (err)
                return nullptr;
            func->AddParam(param);

            token = PeekToken();
            if (token.GetType() == TokenType::S_COMMA)
                ReadToken();
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after function parameters", token);
            return nullptr;
        }

        auto block = AnalyseBlockStmt(func, err, retType, false, false);
        if (err)
            return nullptr;

        func->SetBlockStmt(block);

        return func;
    }

    /*
    <parameter-declaration> ::= 
        [<const-qualifier>]<type-specifier><identifier>
    */
    VarDeclASTPtr Analyser::AnalyseFuncDeclParam(ASTPtr parent, AnalyseError& err)
    {
        auto isConst = false;
        auto token = ReadToken();
        if (token.GetType() == TokenType::R_CONST)
        {
            isConst = true;
            token = ReadToken();
        }

        const auto varType = TokenType2VarType(token.GetType());
        if (VarType::Nul == varType)
        {
            err = AnalyseError("expect type-specifier in function parameter list", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::IDENT)
        {
            err = AnalyseError("expect variable name", token);
            return nullptr;
        }
        const auto varName = token.GetString();
        const auto t = parent->GetSymbolType(varName, false);
        if (t != SymbolType::Nul)
        {
            err = AnalyseError("variable name repeated", token);
            return nullptr;
        }

        return std::make_shared<VarDeclAST>(parent, true, isConst, varType, varName);
    }
}
