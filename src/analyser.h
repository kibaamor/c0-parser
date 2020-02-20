#pragma once
#include "tokenizer.h"
#include "all_ast.h"

namespace c0
{
    class AnalyseError
    {
    public:
        AnalyseError() = default;
        AnalyseError(str_t err, Token token) : _valid(true), _err(err), _token(token) {}

        operator bool() const { return _valid; }

        const str_t& GetError() const { return _err; }
        const Token& GetToken() const { return _token; }
        const str_t& GetSrc() const { return _src; }

        void FixSource(const std::vector<std::string>& lines);

    private:
        bool _valid = false;
        str_t _err;
        Token _token;
        str_t _src;
    };

    class Analyser
    {
    public:
        Analyser(const TokenList& tokens) : _tokens(tokens) {}

        FileASTPtr Analyse(AnalyseError& err);

    private:
        Token PeekToken(size_t offset = 0) const;
        Token ReadToken();
        void UnreadToken(size_t num = 1);
        void SkipSemiColon();

        /*
        <C0-program> ::=
            {<variable-declaration>}{<function-definition>}
        */
        FileASTPtr AnalyseFile(AnalyseError& err);

        //---------------------------------------------------------------------

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
        VarDeclASTPtrList AnalyseVarDecl(ASTPtr parent, AnalyseError& err);
        
        /*
        <function-definition> ::=
            <type-specifier><identifier><parameter-clause><compound-statement>

        <parameter-clause> ::=
            '(' [<parameter-declaration-list>] ')'
        <parameter-declaration-list> ::=
            <parameter-declaration>{','<parameter-declaration>}
        */
        FuncDeclASTPtr AnalyseFuncDecl(ASTPtr parent, AnalyseError& err);

        /*
        <parameter-declaration> ::= 
            [<const-qualifier>]<type-specifier><identifier>
        */
        VarDeclASTPtr AnalyseFuncDeclParam(ASTPtr parent, AnalyseError& err);

        //---------------------------------------------------------------------

        /*
        <expression> ::=
            <additive-expression>
        */
        ExprASTPtr AnalyseExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst);

        /*
        <condition> ::=
            <expression>[<relational-operator><expression>]
        <relational-operator> ::= '<' | '<=' | '>' | '>=' | '!=' | '=='
        */
        BinaryExprASTPtr AnalyseCondExpr(ASTPtr parent, AnalyseError& err);

        /*
        <additive-expression> ::=
            <multiplicative-expression>{<additive-operator><multiplicative-expression>}
        */
        ExprASTPtr AnalyseAddExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst);

        /*
        <multiplicative-expression> ::=
            <cast-expression>{<multiplicative-operator><cast-expression>}
        */
        ExprASTPtr AnalyseMulExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst);

        /*
        <cast-expression> ::=
            {'('<type-specifier>')'}<unary-expression>
        */
        ExprASTPtr AnalyseCastExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst);

        /*
        <unary-expression> ::=
            [<unary-operator>]<primary-expression>
        */
        ExprASTPtr AnalyseUnaryExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst);

        /*
        <primary-expression> ::=
            '('<expression>')'
            |<identifier>
            |<integer-literal>
            |<char-literal>
            |<floating-literal>
            |<function-call>
        */
        ExprASTPtr AnalysePrimaryExpr(ASTPtr parent, AnalyseError& err, bool isNeedConst);

        /*
        <assignment-expression> ::=
            <identifier><assignment-operator><expression>
        <assignment-operator>     ::= '='
        */
        AssignExprASTPtr AnalyseAssignExpr(ASTPtr parent, AnalyseError& err);

        /*
        <function-call> ::= 
            <identifier> '(' [<expression-list>] ')'
        <expression-list> ::= 
            <expression>{','<expression>}
        */
        ExprASTPtr AnalyseFuncCallExpr(ASTPtr parent, AnalyseError& err, bool isNeedReturn);

        //---------------------------------------------------------------------

        /*
        <compound-statement> ::= 
            '{' {<variable-declaration>} <statement-seq> '}'
        <statement-seq> ::= 
            {<statement>}
        
        */
        BlockStmtASTPtr AnalyseBlockStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue);

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
        StmtASTPtr AnalyseStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue, bool* isSemicolon = nullptr);

        /*
        <condition-statement> ::=
             'if' '(' <condition> ')' <statement> ['else' <statement>]
            |'switch' '(' <expression> ')' '{' {<labeled-statement>} '}'
        */
        CondStmtASTPtr AnalyseCondStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue);

        /*
        <loop-statement> ::=
            'while' '(' <condition> ')' <statement>
           |'do' <statement> 'while' '(' <condition> ')' ';'
           |'for' '('<for-init-statement> [<condition>]';' [<for-update-expression>]')' <statement>
        */
        LoopStmtASTPtr AnalyseLoopStmt(ASTPtr parent, AnalyseError& err, VarType retType);

        /*
        <jump-statement> ::=
            'break' ';'
            |'continue' ';'
            |<return-statement>
        */
        JumpStmtASTPtr AnalyseJumpStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue);

        /*
        <print-statement> ::= 'print' '(' [<printable-list>] ')' ';'
        <printable-list>  ::= <printable> {',' <printable>}
        <printable> ::=
            <expression> | <string-literal>
        */
        PrintStmtASTPtr AnalysePrintStmt(ASTPtr parent, AnalyseError& err);

        /*
        <scan-statement>  ::= 'scan' '(' <identifier> ')' ';'
        */
        ScanStmtASTPtr AnalyseScanStmt(ASTPtr parent, AnalyseError& err);

        /*
        <assignment-expression> ::= 
            <identifier><assignment-operator><expression>
        <assignment-operator> ::= '='
        */
        AssignStmtASTPtr AnalyseAssignStmt(ASTPtr parent, AnalyseError& err);

        /*
        <function-call> ::= 
            <identifier> '(' [<expression-list>] ')'
        <expression-list> ::= 
            <expression>{','<expression>}
        */
        FuncCallStmtASTPtr AnalyseFuncCallStmt(ASTPtr parent, AnalyseError& err);

        //---------------------------------------------------------------------

        /*
        'if' '(' <condition> ')' <statement> ['else' <statement>]
        */
        IfStmtASTPtr AnalyseIfStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canBreak, bool canContinue);

        /*
        'switch' '(' <expression> ')' '{' {<labeled-statement>} '}'
        */
        SwitchStmtASTPtr AnalyseSwitchStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canContinue);

        /*
        <labeled-statement> ::= 
             'case' (<integer-literal>|<char-literal>) ':' <statement>
            |'default' ':' <statement>
        */
        StmtASTPtr AnalyseLabeledStmt(ASTPtr parent, AnalyseError& err, VarType retType, bool canContinue);

        /*
        'while' '(' <condition> ')' <statement>
        */
        WhileStmtASTPtr AnalyseWhileStmt(ASTPtr parent, AnalyseError& err, VarType retType);

        /*
        'do' <statement> 'while' '(' <condition> ')' ';'
        */
        DoStmtASTPtr AnalyseDoStmt(ASTPtr parent, AnalyseError& err, VarType retType);

        /*
        'for' '('<for-init-statement> [<condition>]';' [<for-update-expression>]')' <statement>
        <for-init-statement> ::= 
            [<assignment-expression>{','<assignment-expression>}]';'
        <for-update-expression> ::=
            (<assignment-expression>|<function-call>){','(<assignment-expression>|<function-call>)}
        */
        ForStmtASTPtr AnalyseForStmt(ASTPtr parent, AnalyseError& err, VarType retType);

        /*
        'break' ';'
        */
        BreakStmtASTPtr AnalyseBreakStmt(ASTPtr parent, AnalyseError& err, bool canBreak);

        /*
        'continue' ';'
        */
        ContinueStmtASTPtr AnalyseContinueStmt(ASTPtr parent, AnalyseError& err, bool canContinue);

        /*
        <return-statement> ::= 'return' [<expression>] ';'
        */
        ReturnStmtASTPtr AnalyseReturnStmt(ASTPtr parent, AnalyseError& err, VarType retType);

    private:
        ExprASTPtr CheckInexplicitTypeCast(ASTPtr parent, AnalyseError& err, const Token& token,
            ExprASTPtr fromExpr, VarType toType, const str_t& extralog);

    private:
        const TokenList _tokens;
        std::size_t _cur = 0;
    };
}

namespace std
{
    string to_string(const c0::AnalyseError& err);
}