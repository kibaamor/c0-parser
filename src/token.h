#pragma once
#include <cstdint>
#include <string>

namespace c0
{
    enum class TokenType : std::uint8_t
    {
        NUL,

        ERR,
        IDENT,
        INT, CHAR, STR, FLOAT,

        R_CONST, R_VOID, R_INT, R_CHAR, R_DOUBLE, R_STRUCT, R_IF, R_ELSE,
        R_SWITCH, R_CASE, R_DEFAULT, R_WHILE, R_FOR, R_DO, R_RETURN,
        R_BREAK, R_CONTINUE, R_PRINT, R_SCAN,

        // < = >  ( )  { } , : ; ! + - * /
        O_LESS,         // <
        O_GREATER,      // >
        O_LESSEQUAL,    // <=
        O_GREATERQUAL,  // >=
        O_EQUAL,        // ==
        O_NOTEUQAL,     // !=

        S_ASSIGN,       // =
        S_LBRACES,      // (
        S_RBRACES,      // )
        S_LPARENTHESES, // {
        S_RPARENTHESES, // }
        S_COMMA,        // ,
        S_COLON,        // :
        S_SEMICOLON,    // ;
        S_EXCALMATION,  // !
        S_PLUS,         // +
        S_MINUS,        // -
        S_MUL,          // *
        S_DIV,          // /
    };

    using char_t = char;
    using str_t = std::string;
    using int_t = int32_t;
    using float_t = double;
    using pos_t = std::pair<std::size_t, std::size_t>;
    using posrange_t = std::pair<pos_t, pos_t>;

    inline str_t Char2String(char_t c) { return str_t(1, c); }

    class Token
    {
    public:
        static bool IsSign(char_t c);
        static Token Parse(str_t s, const posrange_t& posrange);
        static Token Error(str_t s, const posrange_t& posrange);

    public:
        Token() = default;
        Token(TokenType t, str_t s, const posrange_t& posrange) : _type(t), _string(s), _posrange(posrange) {}
        Token(int_t i, const posrange_t& posrange) : _type(TokenType::INT), _int(i), _posrange(posrange) {}
        Token(char_t c, const posrange_t& posrange) : _type(TokenType::CHAR), _char(c), _posrange(posrange) {}
        Token(float_t f, const posrange_t& posrange) : _type(TokenType::FLOAT), _float(f), _posrange(posrange) {}

        void SetPosRange(const posrange_t& posrange) { _posrange = posrange; }

        TokenType GetType() const { return _type; }
        const str_t& GetString() const { return _string; }
        int_t GetInt() const { return _int; }
        char_t GetChar() const { return _char; }
        float_t GetFloat() const { return _float; }
        const posrange_t& GetPosRange() const { return _posrange; }
        str_t GetValueString() const;
        bool IsNul() const { return _type == TokenType::NUL; }
        bool IsError() const { return _type == TokenType::ERR; }
        bool IsSimpleTypeSpecifier(bool includeVoid) const;
        
    private:
        TokenType _type = TokenType::NUL;
        str_t _string;
        int_t _int = 0;
        char_t _char = 0;
        float_t _float = 0.0;
        posrange_t _posrange;
    };
}

namespace std
{
    string to_string(c0::TokenType t);
    string to_string(const c0::pos_t& pos);
    string to_string(const c0::posrange_t& posrange);
    string to_string(const c0::Token& token);
}
