#include "analyser.h"

namespace c0
{
    /*
    <expression> ::=
        <additive-expression>
    */
    ExprASTPtr Analyser::AnalyseExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst)
    {
        return AnalyseAddExpr(parent, err, isNeedConst);
    }

    /*
    <condition> ::=
        <expression>[<relational-operator><expression>]
    <relational-operator> ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
    */
    BinaryExprASTPtr Analyser::AnalyseCondExpr(ASTPtr parent, AnalyseError& err)
    {
        auto left = AnalyseExpr(parent, err, false);
        if (err)
            return nullptr;

        BinaryType bt = BinaryType::Nul;
        auto token = ReadToken();
        switch (token.GetType())
        {
        case TokenType::O_LESS: bt = BinaryType::Less; break;
        case TokenType::O_LESSEQUAL: bt = BinaryType::LessEqual; break;
        case TokenType::O_GREATER: bt = BinaryType::Greater; break;
        case TokenType::O_GREATERQUAL: bt = BinaryType::GreaterEqual; break;
        case TokenType::O_NOTEUQAL: bt = BinaryType::NotEqual; break;
        case TokenType::O_EQUAL: bt = BinaryType::Euqal; break;
        }

        ExprASTPtr right;
        if (BinaryType::Nul != bt)
        {
            right = AnalyseExpr(parent, err, false);
            if (err)
                return nullptr;
        }
        else
        {
            UnreadToken();

            bt = BinaryType::NotEqual;
            if (left->GetVarType() != VarType::Float)
                right = std::make_shared<IntExprAST>(parent, 0);
            else
                right = std::make_shared<FloatExprAST>(parent, 0.0);
        }

        const auto varType = MergeVarType(left->GetVarType(), right->GetVarType());
        left = CheckInexplicitTypeCast(parent, err, token, left, varType, "");
        if (err)
            return nullptr;
        right = CheckInexplicitTypeCast(parent, err, token, right, varType, "");
        if (err)
            return nullptr;

        auto expr = std::make_shared<BinaryExprAST>(parent, left, bt, right);
        left->SetParent(expr);
        right->SetParent(expr);
        return expr;
    }

    /*
    <additive-expression> ::=
        <multiplicative-expression>{<additive-operator><multiplicative-expression>}
    */
    ExprASTPtr Analyser::AnalyseAddExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst)
    {
        auto left = AnalyseMulExpr(parent, err, isNeedConst);
        if (err)
            return nullptr;

        for (auto token = PeekToken();
            token.GetType() == TokenType::S_PLUS || token.GetType() == TokenType::S_MINUS;
            token = PeekToken())
        {
            ReadToken();
            const auto bt = (token.GetType() == TokenType::S_PLUS ? BinaryType::Add : BinaryType::Sub);

            auto right = AnalyseMulExpr(parent, err, isNeedConst);
            if (err)
                return nullptr;

            const auto varType = MergeVarType(left->GetVarType(), right->GetVarType());
            left = CheckInexplicitTypeCast(parent, err, token, left, varType, "");
            if (err)
                return nullptr;
            right = CheckInexplicitTypeCast(parent, err, token, right, varType, "");
            if (err)
                return nullptr;

            auto expr = std::make_shared<BinaryExprAST>(parent, left, bt, right);
            expr->GetLeftExpr()->SetParent(expr);
            expr->GetRightExpr()->SetParent(expr);
            left = expr;
        }

        return left;
    }
    
    /*
    <multiplicative-expression> ::=
        <cast-expression>{<multiplicative-operator><cast-expression>}
    */
    ExprASTPtr Analyser::AnalyseMulExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst)
    {
        auto left = AnalyseCastExpr(parent, err, isNeedConst);
        if (err)
            return nullptr;

        for (auto token = PeekToken();
            token.GetType() == TokenType::S_MUL || token.GetType() == TokenType::S_DIV;
            token = PeekToken())
        {
            ReadToken();
            const auto bt = (token.GetType() == TokenType::S_MUL ? BinaryType::Mul : BinaryType::Div);

            auto right = AnalyseCastExpr(parent, err, isNeedConst);
            if (err)
                return nullptr;

            const auto varType = MergeVarType(left->GetVarType(), right->GetVarType());
            left = CheckInexplicitTypeCast(parent, err, token, left, varType, "");
            if (err)
                return nullptr;
            right = CheckInexplicitTypeCast(parent, err, token, right, varType, "");
            if (err)
                return nullptr;

            auto expr = std::make_shared<BinaryExprAST>(parent, left, bt, right);
            expr->GetLeftExpr()->SetParent(expr);
            expr->GetRightExpr()->SetParent(expr);
            left = expr;
        }

        return left;
    }

    /*
    <cast-expression> ::=
        {'('<type-specifier>')'}<unary-expression>
    */
    ExprASTPtr Analyser::AnalyseCastExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst)
    {
        VarType varType = VarType::Nul;
        auto token = PeekToken();
        if (token.GetType() == TokenType::S_LBRACES)
        {
            token = PeekToken(1);
            varType = TokenType2VarType(token.GetType());
            if (IsValidCastType(varType))
            {
                ReadToken();
                ReadToken();
                token = ReadToken();
                if (token.GetType() != TokenType::S_RBRACES)
                {
                    err = AnalyseError("invalid cast expression, expect ')' after type", token);
                    return nullptr;
                }
            }
        }

        auto expr = AnalyseUnaryExpr(parent, err, isNeedConst);
        if (err)
            return nullptr;

        if (VarType::Nul != varType)
        {
            if (!IsVarTypeCastable(expr->GetVarType(), varType))
            {
                err = AnalyseError("can not cast type from '"
                    + std::to_string(expr->GetVarType()) + "' to '"
                    + std::to_string(varType) + "'", token);
                return nullptr;
            }
            if (IsValidCastType(varType))
            {
                auto cast = std::make_shared<CastExprAST>(parent, expr, varType, true);
                cast->GetExpr()->SetParent(cast);
                expr = cast;
            }
        }
        return expr;
    }

    /*
    <unary-expression> ::=
        [<unary-operator>]<primary-expression>
    */
    ExprASTPtr Analyser::AnalyseUnaryExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst)
    {
        auto token = PeekToken();
        if (token.GetType() == TokenType::S_PLUS || token.GetType() == TokenType::S_MINUS)
        {
            ReadToken();
            const auto ut = (token.GetType() == TokenType::S_PLUS ? UnaryType::Positive : UnaryType::Negative);

            auto expr = AnalysePrimaryExpr(parent, err, isNeedConst);
            if (err)
                return nullptr;

            if (expr->GetVarType() == VarType::Str)
            {
                err = AnalyseError("cannot apply unary operator on string", token);
                return nullptr;
            }

            auto tmp = std::make_shared<UnaryExprAST>(parent, ut, expr);
            tmp->GetExpr()->SetParent(tmp);
            return tmp;
        }
        return AnalysePrimaryExpr(parent, err, isNeedConst);
    }
    
    /*
    <primary-expression> ::=
        '('<expression>')'
        |<identifier>
        |<integer-literal>
        |<char-literal>
        |<floating-literal>
        |<function-call>
    */
    ExprASTPtr Analyser::AnalysePrimaryExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst)
    {
        auto token = ReadToken();
        if (token.GetType() == TokenType::S_LBRACES)
        {
            auto expr = AnalyseExpr(parent, err, isNeedConst);
            token = ReadToken();
            if (token.GetType() != TokenType::S_RBRACES)
            {
                err = AnalyseError("expect ')' after expression", token);
                return nullptr;
            }
            auto tmp = std::make_shared<BraceExprAST>(parent, expr);
            tmp->GetExpr()->SetParent(tmp);
            return tmp;
        }
        else if (token.GetType() == TokenType::INT)
        {
            return std::make_shared<IntExprAST>(parent, token.GetInt());
        }
        else if (token.GetType() == TokenType::CHAR)
        {
            return std::make_shared<CharExprAST>(parent, token.GetChar());
        }
        else if (token.GetType() == TokenType::FLOAT)
        {
            return std::make_shared<FloatExprAST>(parent, token.GetFloat());
        }
        else if (token.GetType() == TokenType::STR)
        {
            return std::make_shared<StrExprAST>(parent, token.GetString());
        }
        else if (token.GetType() == TokenType::IDENT)
        {
            const auto t = parent->GetSymbolType(token.GetString(), true);
            if (t == SymbolType::Var || t == SymbolType::ConstVar)
            {
                if (isNeedConst && t != SymbolType::ConstVar)
                {
                    err = AnalyseError("expect const variable", token);
                    return nullptr;
                }
                return std::make_shared<IdentExprAST>(parent, token.GetString());
            }
            else if (t == SymbolType::Func)
            {
                if (isNeedConst)
                {
                    err = AnalyseError("expect const express but got function call", token);
                    return nullptr;
                }

                UnreadToken();
                return AnalyseFuncCallExpr(parent, err, true);
            }

            err = AnalyseError("unknown identifier in primary expression", token);
            return nullptr;
        }

        err = AnalyseError("expect primary expression", token);
        return nullptr;
    }

    /*
    <assignment-expression> ::=
        <identifier><assignment-operator><expression>
    <assignment-operator>     ::= '='
    */
    AssignExprASTPtr Analyser::AnalyseAssignExpr(ASTPtr parent, AnalyseError& err)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::IDENT)
        {
            err = AnalyseError("expect identifier in assignment expression", token);
            return nullptr;
        }
        const auto varName = token.GetString();
        auto vardecl = std::dynamic_pointer_cast<VarDeclAST>(parent->GetSymbol(varName, true));
        if (nullptr == vardecl)
        {
            err = AnalyseError("cannot find variable in assignment expression", token);
            return nullptr;
        }
        if (vardecl->IsConst())
        {
            err = AnalyseError("cannot assign on const variable in assignment expression", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_ASSIGN)
        {
            err = AnalyseError("expect '=' after identifier in assignment expression", token);
            return nullptr;
        }

        auto expr = AnalyseExpr(parent, err, false);
        if (err)
            return nullptr;

        expr = CheckInexplicitTypeCast(parent, err, token, expr, vardecl->GetVarType(), 
            "invalid assignment expression, ");
        if (err)
            return nullptr;

        auto tmp = std::make_shared<AssignExprAST>(parent, varName, expr);
        tmp->GetExpr()->SetParent(tmp);
        return tmp;
    }

    /*
    <function-call> ::= 
        <identifier> '(' [<expression-list>] ')'
    <expression-list> ::= 
        <expression>{','<expression>}
    */
    ExprASTPtr Analyser::AnalyseFuncCallExpr(ASTPtr parent, AnalyseError& err, bool isNeedReturn)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::IDENT)
        {
            err = AnalyseError("expect function name in function call expression", token);
            return nullptr;
        }

        const auto funcName = token.GetString();
        const auto funcimpl = std::dynamic_pointer_cast<FuncDeclAST>(parent->GetSymbol(funcName, true));
        if (nullptr == funcimpl)
        {
            err = AnalyseError("identifier is not a function name in function call expression", token);
            return nullptr;
        }
        if (isNeedReturn && funcimpl->GetVarType() == VarType::Void)
        {
            err = AnalyseError("function has no return in function call expression", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' before parameters in function call expression", token);
            return nullptr;
        }

        auto funccall = std::make_shared<FuncCallExprAST>(parent, funcName);
        ExprASTPtrList callParams;

        for (token = PeekToken();
            token.GetType() != TokenType::S_RBRACES;
            token = PeekToken())
        {
            auto param = AnalyseExpr(funccall, err, false);
            if (err)
                return nullptr;
            callParams.push_back(param);
            
            token = PeekToken();
            if (token.GetType() == TokenType::S_COMMA)
                ReadToken();
        }

        const auto& declParams = funcimpl->GetParams();
        if (callParams.size() != declParams.size())
        {
            err = AnalyseError("parameter number mismatch in function call expression, need " 
                + std::to_string(declParams.size())
                + ", have " + std::to_string(callParams.size())
                , token);
            return nullptr;
        }
        for (size_t i = 0, N = callParams.size(); i < N; ++i)
        {
            auto param = CheckInexplicitTypeCast(funccall, err, token, callParams[i], declParams[i]->GetVarType(),
                "for " + std::to_string(i) + "th function param in function call expression, ");
            if (err)
                return nullptr;
            funccall->AddParam(param);
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after parameters in function call expression", token);
            return nullptr;
        }

        return funccall;
    }
}
