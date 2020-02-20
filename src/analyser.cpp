#include "analyser.h"

namespace c0
{
    void AnalyseError::FixSource(const std::vector<std::string>& lines)
    {
        const auto& pos = _token.GetPosRange().first;

        if (pos.first >= lines.size())
            return;

        const auto& line = lines[pos.first];
        if (pos.second >= line.size())
            return;

        _src = line;
    }

    FileASTPtr Analyser::Analyse(AnalyseError& err)
    {
        return AnalyseFile(err);
    }

    Token Analyser::PeekToken(size_t offset) const
    {
        const auto pos = _cur + offset;
        if (pos >= _tokens.size())
            return Token();
        return _tokens[pos];
    }

    Token Analyser::ReadToken()
    {
        const auto token = PeekToken();
        ++_cur;
        return token;
    }

    void Analyser::UnreadToken(size_t num)
    {
        if (_cur > num)
            _cur -= num;
    }

    void Analyser::SkipSemiColon()
    {
        auto token = PeekToken();
        for (; token.GetType() == TokenType::S_SEMICOLON; token = PeekToken())
            ReadToken();
    }
    
    /*
    <C0-program> ::=
        {<variable-declaration>}{<function-definition>}
    */
    FileASTPtr Analyser::AnalyseFile(AnalyseError& err)
    {
        auto file = std::make_shared<FileAST>(nullptr);

        auto canParseVarDecl = true;
        for (auto token = PeekToken(); !token.IsNul(); token = PeekToken())
        {
            if (canParseVarDecl)
            {
                const auto peek = PeekToken(2);
                if (token.GetType() == TokenType::R_CONST
                    || peek.GetType() == TokenType::S_ASSIGN
                    || peek.GetType() == TokenType::S_SEMICOLON
                    || peek.GetType() == TokenType::S_COMMA)
                {
                    auto varlist = AnalyseVarDecl(file, err);
                    if (err)
                        return file;
                    for (const auto& var : varlist)
                        file->AddVar(var);
                    continue;
                }
            }

            canParseVarDecl = false;
            auto func = AnalyseFuncDecl(file, err);
            if (err)
                return file;
            file->AddFunc(func);
        }

        return file;
    }

    ExprASTPtr Analyser::CheckInexplicitTypeCast(ASTPtr parent, AnalyseError& err, const Token& token, 
        ExprASTPtr fromExpr, VarType toType, const str_t& extralog)
    {
        if (fromExpr->GetVarType() == toType)
            return fromExpr;

        if (!IsVarTypeCastable(fromExpr->GetVarType(), toType))
        {
            err = AnalyseError(extralog + "cannot inexplicit cast type from '"
                + std::to_string(fromExpr->GetVarType()) + "' to '"
                + std::to_string(toType) + "'", token);
            return nullptr;
        }
        auto cast = std::make_shared<CastExprAST>(parent, fromExpr, toType, false);
        cast->GetExpr()->SetParent(cast);
        return cast;
    }
}

namespace std
{
    string to_string(const c0::AnalyseError& err)
    {
        const auto token = err.GetToken();
        if (!err)
            return "no error. " + to_string(token);

        return "error: " + err.GetError() + ". " + to_string(token) + "\n"
            + err.GetSrc() + std::string(token.GetPosRange().first.second, ' ') + "^";
    }
}