#include "analyser.h"

namespace c0
{
    /*
    <compound-statement> ::=
        '{' {<variable-declaration>} <statement-seq> '}'
    <statement-seq> ::=
        {<statement>}
    */
    BlockStmtASTPtr Analyser::AnalyseBlockStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::S_LPARENTHESES)
        {
            err = AnalyseError("expect '{' at block begin", token);
            return nullptr;
        }

        auto block = std::make_shared<BlockStmtAST>(parent);

        for (token = PeekToken();
            token.GetType() == TokenType::R_CONST 
            || IsValidCastType(TokenType2VarType(token.GetType()));
            token = PeekToken())
        {
            auto varlist = AnalyseVarDecl(block, err);
            if (err)
                return nullptr;
            for (const auto& var : varlist)
                block->AddVar(var);
        }

        for (token = PeekToken();
            token.GetType() != TokenType::S_RPARENTHESES;
            token = PeekToken())
        {
            bool isSemicolon = false;
            auto stmt = AnalyseStmt(block, err, retType, canBreak, canContinue, &isSemicolon);
            if (err)
                return nullptr;

            if (nullptr != stmt)
                block->AddStmt(stmt);
            else if (!isSemicolon)
                break;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_RPARENTHESES)
        {
            err = AnalyseError("expect '}' at block end", token);
            return block;
        }

        return block;
    }

    /*
    <statement> ::=
        <compound-statement>
        |<condition-statement>
        |<loop-statement>
        |<jump-statement>
        |<print-statement>
        |<scan-statement>
        |<assignment-expression>';'
        |<function-call>';'
        |';'
    */
    StmtASTPtr Analyser::AnalyseStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue, bool* isSemicolon /*= nullptr*/)
    {
        if (nullptr != isSemicolon)
            *isSemicolon = false;
        auto token = PeekToken();
        switch (token.GetType())
        {
        case TokenType::S_SEMICOLON:
            SkipSemiColon();
            if (nullptr != isSemicolon)
                *isSemicolon = true;
            return nullptr;

        case TokenType::S_LPARENTHESES:
            return AnalyseBlockStmt(parent, err, retType, canBreak, canContinue);

        case TokenType::R_IF:
        case TokenType::R_SWITCH:
            return AnalyseCondStmt(parent, err, retType, canBreak, canContinue);

        case TokenType::R_WHILE:
        case TokenType::R_DO:
        case TokenType::R_FOR:
            return AnalyseLoopStmt(parent, err, retType);

        case TokenType::R_BREAK:
        case TokenType::R_CONTINUE:
        case TokenType::R_RETURN:
            return AnalyseJumpStmt(parent, err, retType, canBreak, canContinue);

        case TokenType::R_PRINT:
            return AnalysePrintStmt(parent, err);

        case TokenType::R_SCAN:
            return AnalyseScanStmt(parent, err);

        default:
            break;
        }

        if (token.GetType() == TokenType::IDENT)
        {
            const auto t = parent->GetSymbolType(token.GetString(), true);

            StmtASTPtr stmt = nullptr;
            if (t == SymbolType::Var)
                stmt = AnalyseAssignStmt(parent, err);
            else if (t == SymbolType::Func)
                stmt = AnalyseFuncCallStmt(parent, err);
            else if (t == SymbolType::ConstVar)
                err = AnalyseError("cannot change const variable", token);
            else
                err = AnalyseError("unknown identifier in statement", token);

            if (err)
                return nullptr;

            token = ReadToken();
            if (token.GetType() != TokenType::S_SEMICOLON)
            {
                if (t == SymbolType::Var)
                    err = AnalyseError("expect ';' after assignment", token);
                else
                    err = AnalyseError("expect ';' after function call", token);
                return nullptr;
            }

            return stmt;
        }

        if (token.GetType() != TokenType::R_ELSE)
            err = AnalyseError("invalid statement", token);
        return nullptr;
    }

    /*
    <condition-statement> ::=
        'if' '(' <condition> ')' <statement> ['else' <statement>]
        |'switch' '(' <expression> ')' '{' {<labeled-statement>} '}'
    */
    CondStmtASTPtr Analyser::AnalyseCondStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue)
    {
        auto token = PeekToken();
        if (token.GetType() == TokenType::R_IF)
            return AnalyseIfStmt(parent, err, retType, canBreak, canContinue);
        else if (token.GetType() == TokenType::R_SWITCH)
            return AnalyseSwitchStmt(parent, err, retType, canContinue);
        
        err = AnalyseError("unsupported condition statement", token);
        return nullptr;
    }

    /*
    <loop-statement> ::=
        'while' '(' <condition> ')' <statement>
        |'do' <statement> 'while' '(' <condition> ')' ';'
        |'for' '('<for-init-statement> [<condition>]';' [<for-update-expression>]')' <statement>
    */
    LoopStmtASTPtr Analyser::AnalyseLoopStmt(ASTPtr parent, AnalyseError& err, VarType retType)
    {
        auto token = PeekToken();
        if (token.GetType() == TokenType::R_WHILE)
            return AnalyseWhileStmt(parent, err, retType);
        else if (token.GetType() == TokenType::R_DO)
            return AnalyseDoStmt(parent, err, retType);
        else if (token.GetType() == TokenType::R_FOR)
            return AnalyseForStmt(parent, err, retType);

        err = AnalyseError("unsupported loop statement", token);
        return nullptr;
    }

    /*
    <jump-statement> ::=
        'break' ';'
        |'continue' ';'
        |<return-statement>
    */
    JumpStmtASTPtr Analyser::AnalyseJumpStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue)
    {
        auto token = PeekToken();
        if (token.GetType() == TokenType::R_BREAK)
            return AnalyseBreakStmt(parent, err, canBreak);
        else if (token.GetType() == TokenType::R_CONTINUE)
            return AnalyseContinueStmt(parent, err, canContinue);
        else if (token.GetType() == TokenType::R_RETURN)
            return AnalyseReturnStmt(parent, err, retType);

        err = AnalyseError("unsupported jump statement", token);
        return nullptr;
    }

    /*
    <print-statement> ::= 'print' '(' [<printable-list>] ')' ';'
    <printable-list>  ::= <printable> {',' <printable>}
    <printable> ::=
        <expression> | <string-literal>
    */
    PrintStmtASTPtr Analyser::AnalysePrintStmt(ASTPtr parent, AnalyseError& err)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_PRINT)
        {
            err = AnalyseError("expect 'print'", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' after print", token);
            return nullptr;
        }

        auto print = std::make_shared<PrintStmtAST>(parent);

        for (token = PeekToken();
            token.GetType() != TokenType::S_RBRACES;
            token = PeekToken())
        {
            auto param = AnalyseExpr(parent, err, false);
            if (err)
                return nullptr;
            print->AddParam(param);

            token = PeekToken();
            if (token.GetType() == TokenType::S_COMMA)
                ReadToken();
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after print parameters", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_SEMICOLON)
        {
            err = AnalyseError("expect ';' after scan statement", token);
            return nullptr;
        }

        return print;
    }

    /*
    <scan-statement>  ::= 'scan' '(' <identifier> ')' ';'
    */
    ScanStmtASTPtr Analyser::AnalyseScanStmt(ASTPtr parent, AnalyseError& err)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_SCAN)
        {
            err = AnalyseError("expect 'scan'", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' after scan", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::IDENT)
        {
            err = AnalyseError("expect identifier in scan parameter", token);
            return nullptr;
        }
        const auto varName = token.GetString();

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after scan parameters", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_SEMICOLON)
        {
            err = AnalyseError("expect ';' after scan statement", token);
            return nullptr;
        }

        return std::make_shared<ScanStmtAST>(parent, varName);
    }

    /*
    <assignment-expression> ::= 
        <identifier><assignment-operator><expression>
    <assignment-operator> ::= '=' 
    */
    AssignStmtASTPtr Analyser::AnalyseAssignStmt(ASTPtr parent, AnalyseError& err)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::IDENT)
        {
            err = AnalyseError("expect identifier in assignment statement", token);
            return nullptr;
        }
        const auto varName = token.GetString();
        auto vardecl = std::dynamic_pointer_cast<VarDeclAST>(parent->GetSymbol(varName, true));
        if (nullptr == vardecl)
        {
            err = AnalyseError("cannot find variable in assignment statement", token);
            return nullptr;
        }
        if (vardecl->IsConst())
        {
            err = AnalyseError("cannot assign on const variable in assignment statement", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_ASSIGN)
        {
            err = AnalyseError("expect '=' after identifier in assignment statement", token);
            return nullptr;
        }

        auto expr = AnalyseExpr(parent, err, false);
        if (err)
            return nullptr;

        expr = CheckInexplicitTypeCast(parent, err, token, expr, vardecl->GetVarType(),
            "invalid assignment statement, ");
        if (err)
            return nullptr;

        auto tmp = std::make_shared<AssignStmtAST>(parent, varName, expr);
        tmp->GetExpr()->SetParent(tmp);
        return tmp;
    }

    /*
    <function-call> ::= 
        <identifier> '(' [<expression-list>] ')'
    <expression-list> ::= 
        <expression>{','<expression>}
    */
    FuncCallStmtASTPtr Analyser::AnalyseFuncCallStmt(ASTPtr parent, AnalyseError& err)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::IDENT)
        {
            err = AnalyseError("expect identifier in function call statement", token);
            return nullptr;
        }
        const auto funcName = token.GetString();
        const auto funcimpl = std::dynamic_pointer_cast<FuncDeclAST>(parent->GetSymbol(funcName, true));
        if (nullptr == funcimpl)
        {
            err = AnalyseError("identifier is not a function name in function call statement", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' before function call parameters in function call statement", token);
            return nullptr;
        }

        auto funccall = std::make_shared<FuncCallStmtAST>(parent, funcName);
        ExprASTPtrList callParams;

        for (token = PeekToken();
            token.GetType() != TokenType::S_RBRACES;
            token = PeekToken())
        {
            auto param = AnalyseExpr(parent, err, false);
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
            err = AnalyseError("parameter number mismatch in function call statement, need "
                + std::to_string(declParams.size())
                + ", have " + std::to_string(callParams.size())
                , token);
            return nullptr;
        }
        for (size_t i = 0, N = callParams.size(); i < N; ++i)
        {
            auto param = CheckInexplicitTypeCast(funccall, err, token, callParams[i], declParams[i]->GetVarType(),
                "for " + std::to_string(i) + "th function param in function call statement, ");
            if (err)
                return nullptr;
            funccall->AddParam(param);
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after parameters in function call statement", token);
            return nullptr;
        }

        return funccall;
    }

    /*
    'if' '(' <condition> ')' <statement> ['else' <statement>]
    */
    IfStmtASTPtr Analyser::AnalyseIfStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_IF)
        {
            err = AnalyseError("expect 'if'", token);
            return nullptr;
        }
        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' before 'if'", token);
            return nullptr;
        }

        auto ifcond = AnalyseCondExpr(parent, err);
        if (err)
            return nullptr;

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after 'if'", token);
            return nullptr;
        }

        auto ifstmt = AnalyseStmt(parent, err, retType, canBreak, canContinue);
        if (err)
            return nullptr;
        if (nullptr == ifstmt)
            ifstmt = std::make_shared<EmptyStmtAST>(parent);
        auto ifptr = std::make_shared<IfStmtAST>(parent, ifcond, ifstmt);
        ifcond->SetParent(ifptr);
        ifstmt->SetParent(ifptr);

        token = PeekToken();
        if (token.GetType() == TokenType::R_ELSE)
        {
            ReadToken();

            auto elsestat = AnalyseStmt(ifptr, err, retType, canBreak, canContinue);
            if (err)
                return nullptr;

            ifptr->SetElseStmt(elsestat);
        }

        return ifptr;
    }

    /*
    'switch' '(' <expression> ')' '{' {<labeled-statement>} '}'
    */
    SwitchStmtASTPtr Analyser::AnalyseSwitchStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canContinue)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_SWITCH)
        {
            err = AnalyseError("expect 'switch'", token);
            return nullptr;
        }
        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' before 'switch' expression", token);
            return nullptr;
        }

        auto cond = AnalyseExpr(parent, err, false);
        if (err)
            return nullptr;
        const auto condVarType = cond->GetVarType();
        if (!IsValidCastType(condVarType))
        {
            err = AnalyseError("invalid switch condition expression type:" + std::to_string(condVarType), token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after 'switch' expression", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_LPARENTHESES)
        {
            err = AnalyseError("expect '{' at switch case begin", token);
            return nullptr;
        }

        StmtASTPtrList caseStmts;
        auto hasDefault = false;
        for (token = PeekToken();
            token.GetType() == TokenType::R_CASE || token.GetType() == TokenType::R_DEFAULT;
            token = PeekToken())
        {
            auto stmt = AnalyseLabeledStmt(parent, err, retType, canContinue);
            if (err)
                return nullptr;

            if (stmt->GetASTType() != ASTType::LabeledStmt)
            {
                if (hasDefault)
                {
                    err = AnalyseError("only one 'default' case allowed", token);
                    return nullptr;
                }
                hasDefault = true;
            }
            caseStmts.push_back(stmt);
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_RPARENTHESES)
        {
            err = AnalyseError("expect '}' at switch case end", token);
            return nullptr;
        }

        auto switchptr = std::make_shared<SwitchStmtAST>(parent, cond);
        switchptr->GetExpr()->SetParent(switchptr);
        for (const auto& stmt : caseStmts)
        {
            stmt->SetParent(switchptr);
            switchptr->AddStmt(stmt);
        }
        return switchptr;
    }

    /*
    <labeled-statement> ::=
         'case' (<integer-literal>|<char-literal>) ':' <statement>
        |'default' ':' <statement>
    */
    StmtASTPtr Analyser::AnalyseLabeledStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canContinue)
    {
        auto isCase = false;
        int_t i;

        auto token = ReadToken();
        if (token.GetType() == TokenType::R_CASE)
        {
            auto expr = AnalyseExpr(parent, err, true);
            if (err)
                return nullptr;
            if (expr->GetASTType() == ASTType::IntExpr)
            {
                i = expr->GetInt();
                isCase = true;
            }
            else if (expr->GetASTType() == ASTType::CharExpr)
            {
                i = int_t(expr->GetChar());
                isCase = true;
            }
            else
            {
                err = AnalyseError("switch cast expression only can be integer literal or char literal", token);
                return nullptr;
            }
        }
        else if (token.GetType() == TokenType::R_DEFAULT)
        {
        }
        else
        {
            err = AnalyseError("invalid switch cast", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_COLON)
        {
            if (isCase)
                err = AnalyseError("expect ':' after 'cast' condition", token);
            else
                err = AnalyseError("expect ':' after 'default'", token);
            return nullptr;
        }

        auto stmt = AnalyseStmt(parent, err, retType, true, canContinue, nullptr);
        if (err)
            return nullptr;

        if (!isCase)
            return stmt;

        auto caseStmt = std::make_shared<LabeledStmtAST>(parent, i, stmt);
        caseStmt->GetStmt()->SetParent(caseStmt);
        return caseStmt;
    }

    /*
    'while' '(' <condition> ')' <statement>
    */
    WhileStmtASTPtr Analyser::AnalyseWhileStmt(ASTPtr parent, AnalyseError& err, VarType retType)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_WHILE)
        {
            err = AnalyseError("expect 'while'", token);
            return nullptr;
        }
        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' after 'while'", token);
            return nullptr;
        }

        auto cond = AnalyseCondExpr(parent, err);
        if (err)
            return nullptr;

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after 'while'", token);
            return nullptr;
        }

        auto stmt = AnalyseStmt(parent, err, retType, true, true);
        if (err)
            return nullptr;
        if (nullptr == stmt)
            stmt = std::make_shared<EmptyStmtAST>(parent);
        auto tmp = std::make_shared<WhileStmtAST>(parent, cond, stmt);
        tmp->GetCond()->SetParent(tmp);
        tmp->GetStmt()->SetParent(tmp);
        return tmp;
    }

    /*
    'do' <statement> 'while' '(' <condition> ')' ';'
    */
    DoStmtASTPtr Analyser::AnalyseDoStmt(ASTPtr parent, AnalyseError& err, VarType retType)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_DO)
        {
            err = AnalyseError("expect 'do'", token);
            return nullptr;
        }

        auto stmt = AnalyseStmt(parent, err, retType, true, true);
        if (err)
            return nullptr;
        if (nullptr == stmt)
            stmt = std::make_shared<EmptyStmtAST>(parent);

        token = ReadToken();
        if (token.GetType() != TokenType::R_WHILE)
        {
            err = AnalyseError("expect 'while' after do statement", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' before 'do while' condition", token);
            return nullptr;
        }

        auto cond = AnalyseCondExpr(parent, err);
        if (err)
            return nullptr;

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after 'do while' condition", token);
            return nullptr;
        }

        auto tmp = std::make_shared<DoStmtAST>(parent, stmt, cond);
        tmp->GetStmt()->SetParent(tmp);
        tmp->GetCond()->SetParent(tmp);
        return tmp;
    }

    /*
    'for' '('<for-init-statement> [<condition>]';' [<for-update-expression>]')' <statement>
    <for-init-statement> ::=
        [<assignment-expression>{','<assignment-expression>}]';'
    <for-update-expression> ::=
        (<assignment-expression>|<function-call>){','(<assignment-expression>|<function-call>)}
    */
    ForStmtASTPtr Analyser::AnalyseForStmt(ASTPtr parent, AnalyseError& err, VarType retType)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_FOR)
        {
            err = AnalyseError("expect 'for'", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_LBRACES)
        {
            err = AnalyseError("expect '(' after 'for'", token);
            return nullptr;
        }
        auto forptr = std::make_shared<ForStmtAST>(parent);

        for (token = PeekToken();
            token.GetType() != TokenType::S_SEMICOLON;
            token = PeekToken())
        {
            auto expr = AnalyseAssignExpr(forptr, err);
            if (err)
                return nullptr;
            forptr->AddInitExpr(expr);

            token = PeekToken();
            if (token.GetType() == TokenType::S_COMMA)
                ReadToken();
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_SEMICOLON)
        {
            err = AnalyseError("expect ';' after for init expression", token);
            return nullptr;
        }

        const auto readPos = _cur;
        auto cond = AnalyseCondExpr(forptr, err);
        if (err || PeekToken().GetType() != TokenType::S_SEMICOLON)
        {
            _cur = readPos;
            err = AnalyseError();

            auto left = std::make_shared<IntExprAST>(forptr, 1);
            auto right = std::make_shared<IntExprAST>(forptr, 0);
            cond = std::make_shared<BinaryExprAST>(forptr, left, BinaryType::NotEqual, right);
            cond->GetLeftExpr()->SetParent(cond);
            cond->GetRightExpr()->SetParent(cond);
        }
        else
        {
            token = ReadToken();
            if (token.GetType() != TokenType::S_SEMICOLON)
            {
                err = AnalyseError("expect ';' after for condition", token);
                return nullptr;
            }
        }
        forptr->SetCond(cond);

        for (token = PeekToken();
            token.GetType() != TokenType::S_RBRACES;
            token = PeekToken())
        {
            if (token.GetType() != TokenType::IDENT)
            {
                err = AnalyseError("invalid for update express", token);
                return nullptr;
            }
            const auto name = token.GetString();

            ExprASTPtr expr;
            if (parent->GetSymbolType(name, true) == SymbolType::Func)
                expr = AnalyseFuncCallExpr(forptr, err, false);
            else
                expr = AnalyseAssignExpr(forptr, err);
            if (err)
                return nullptr;
            forptr->AddUpdateExpr(expr);

            token = PeekToken();
            if (token.GetType() == TokenType::S_COMMA)
                ReadToken();
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_RBRACES)
        {
            err = AnalyseError("expect ')' after for update expression", token);
            return nullptr;
        }

        auto body = AnalyseStmt(forptr, err, retType, true, true);
        if (err)
            return nullptr;
        if (nullptr == body)
            body = std::make_shared<EmptyStmtAST>(forptr);
        forptr->SetBody(body);

        return forptr;
    }

    /*
    'break' ';'
    */
    BreakStmtASTPtr Analyser::AnalyseBreakStmt(ASTPtr parent, AnalyseError& err, bool canBreak)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_BREAK)
        {
            err = AnalyseError("expect 'break'", token);
            return nullptr;
        }
        if (!canBreak)
        {
            err = AnalyseError("only loop or switch can use 'break' statement", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_SEMICOLON)
        {
            err = AnalyseError("expect ';' after 'break'", token);
            return nullptr;
        }

        return std::make_shared<BreakStmtAST>(parent);
    }

    /*
    'continue' ';'
    */
    ContinueStmtASTPtr Analyser::AnalyseContinueStmt(ASTPtr parent, AnalyseError& err, bool canContinue)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_CONTINUE)
        {
            err = AnalyseError("expect 'continue'", token);
            return nullptr;
        }
        if (!canContinue)
        {
            err = AnalyseError("only loop can use 'continue' statement", token);
            return nullptr;
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_SEMICOLON)
        {
            err = AnalyseError("expect ';' after 'continue'", token);
            return nullptr;
        }

        return std::make_shared<ContinueStmtAST>(parent);
    }

    /*
    <return-statement> ::= 'return' [<expression>] ';'
    */
    ReturnStmtASTPtr Analyser::AnalyseReturnStmt(ASTPtr parent, AnalyseError& err, VarType retType)
    {
        auto token = ReadToken();
        if (token.GetType() != TokenType::R_RETURN)
        {
            err = AnalyseError("expect 'return'", token);
            return nullptr;
        }

        auto ret = std::make_shared<ReturnStmtAST>(parent);

        token = PeekToken();
        if (token.GetType() != TokenType::S_SEMICOLON)
        {
            if (VarType::Void == retType)
            {
                err = AnalyseError("void function cannot return any value", token);
                return ret;
            }
            auto expr = AnalyseExpr(parent, err, false);
            if (err)
                return nullptr;

            expr = CheckInexplicitTypeCast(parent, err, token, expr, retType, "");
            if (err)
                return nullptr;

            ret->SetExpr(expr);
        }

        token = ReadToken();
        if (token.GetType() != TokenType::S_SEMICOLON)
        {
            err = AnalyseError("expect ';' after return expression", token);
            return ret;
        }

        return ret;
    }
}