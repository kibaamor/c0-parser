#include "token.h"
#include <unordered_map>

namespace c0
{
    std::unordered_map<std::string, TokenType> dict = 
    {
        {"const", TokenType::R_CONST},
        {"void", TokenType::R_VOID},
        {"int", TokenType::R_INT},
        {"char", TokenType::R_CHAR},
        {"double", TokenType::R_DOUBLE},
        {"struct", TokenType::R_STRUCT},
        {"if", TokenType::R_IF},
        {"else", TokenType::R_ELSE},
        {"switch", TokenType::R_SWITCH},
        {"case", TokenType::R_CASE},
        {"default", TokenType::R_DEFAULT},
        {"while", TokenType::R_WHILE},
        {"for", TokenType::R_FOR},
        {"do", TokenType::R_DO},
        {"return", TokenType::R_RETURN},
        {"break", TokenType::R_BREAK},
        {"continue", TokenType::R_CONTINUE},
        {"print", TokenType::R_PRINT},
        {"scan", TokenType::R_SCAN},

        {"<", TokenType::O_LESS},
        {">", TokenType::O_GREATER},
        {"<=", TokenType::O_LESSEQUAL},
        {">=", TokenType::O_GREATERQUAL},
        {"==", TokenType::O_EQUAL},
        {"!=", TokenType::O_NOTEUQAL},

        {"=", TokenType::S_ASSIGN},
        {"(", TokenType::S_LBRACES},
        {")", TokenType::S_RBRACES},
        {"{", TokenType::S_LPARENTHESES},
        {"}", TokenType::S_RPARENTHESES},
        {",", TokenType::S_COMMA},
        {":", TokenType::S_COLON},
        {";", TokenType::S_SEMICOLON},
        {"!", TokenType::S_EXCALMATION},
        {"+", TokenType::S_PLUS},
        {"-", TokenType::S_MINUS},
        {"*", TokenType::S_MUL},
        {"/", TokenType::S_DIV},
    };

    bool Token::IsSign(char_t c)
    {
        return dict.find(Char2String(c)) != dict.end();
    }

    Token Token::Parse(str_t s, const posrange_t& posrange)
    {
        auto iter = dict.find(s);
        const auto t = (iter != dict.end() ? iter->second : TokenType::IDENT);
        return Token(t, s, posrange);
    }

    Token Token::Error(str_t s, const posrange_t& posrange)
    {
        return Token(TokenType::ERR, s, posrange);
    }

    str_t Token::GetValueString() const
    {
        if (GetType() == TokenType::INT)
        {
            return std::to_string(_int);
        }
        else if (GetType() == TokenType::CHAR)
        {
            str_t s = "'";
            switch (_char)
            {
            case '\\': s += R"(\\)"; break;
            case '\'': s += R"(\')"; break;
            case '\"': s += R"(\")"; break;
            case '\n': s += R"(\n)"; break;
            case '\r': s += R"(\r)"; break;
            case '\t': s += R"(\t)"; break;
            default: s.push_back(_char); break;
            }
            s += "'";
            return s;
        }
        else if (GetType() == TokenType::FLOAT)
        {
            return std::to_string(_float);
        }
        else if (GetType() == TokenType::STR)
        {
            return "\"" + _string + "\"";
        }
        return _string;
    }

    bool Token::IsSimpleTypeSpecifier(bool includeVoid) const
    {
        return (includeVoid ? GetType() == TokenType::R_VOID : false)
            || GetType() == TokenType::R_INT
            || GetType() == TokenType::R_CHAR
            || GetType() == TokenType::R_DOUBLE;
    }
}

namespace std
{
    string to_string(c0::TokenType t)
    {
        using namespace c0;
        if (t == TokenType::NUL)
            return "NUL";
        else if (t == TokenType::ERR)
            return "ERR";
        else if (t == TokenType::IDENT)
            return "IDENT";
        else if (t == TokenType::INT)
            return "INT";
        else if (t == TokenType::CHAR)
            return "CHAR";
        else if (t == TokenType::STR)
            return "STR";
        else if (t == TokenType::FLOAT)
            return "FLOAT";
        else if (t >= TokenType::R_CONST && t <= TokenType::R_SCAN)
            return "RESERVE";
        else if (t >= TokenType::O_LESS && t <= TokenType::O_NOTEUQAL)
            return "OPERATOR";
        else if (t >= TokenType::S_ASSIGN && t <= TokenType::S_DIV)
            return "SIGN";
        else
            return "INVALID";
    }

    string to_string(const c0::pos_t& pos)
    {
        return to_string(pos.first + 1) + ":" + to_string(pos.second + 1);
    }

    string to_string(const c0::posrange_t& posrange)
    {
        return "[" + to_string(posrange.first) + " - " + to_string(posrange.second) + ")";
    }

    string to_string(const c0::Token& token)
    {
        return to_string(token.GetType()) + ":" + token.GetValueString() + " at " + to_string(token.GetPosRange());
    }
}