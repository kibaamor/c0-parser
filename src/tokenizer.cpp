#include "tokenizer.h"
#include <cctype>
#include <iomanip>
#include <iostream>

namespace c0
{
    Tokenizer::Tokenizer(std::istream & stream)
    {
        for (std::string s; std::getline(stream, s);)
            _lines.emplace_back(std::move(s + "\n"));
        _pos = std::make_pair(0, 0);
    }

    void Tokenizer::Dump(std::ostream& stream) const
    {
        for (size_t i = 0, N = _lines.size(); i < N; ++i)
            stream << std::setw(3) << i + 1 << ": " << _lines[i];
    }

    void Tokenizer::Dump(const pos_t& pos, std::ostream& stream) const
    {
        if (pos.first >= _lines.size())
        {
            stream << "invalid row position" << std::endl;
            return;
        }

        const auto& line = _lines[pos.first];
        if (pos.second >= line.size())
        {
            stream << "invalid column position" << std::endl;
            return;
        }

        stream << line;
        if (pos.second > 0)
            stream << str_t(pos.second, ' ');
        stream << "^" << std::endl;
    }

    Token Tokenizer::Next()
    {
        PushPos();
        auto c = ReadChar();
        for (; std::isspace(c); c = ReadChar())
            PushPos();

        if (c == 0)
            return Token();

        const auto p = PeekChar();
        if (std::isdigit(c) != 0 || (c == '.' && std::isdigit(p) != 0))
        {
            UnreadChar();
            return ParseDigit();
        }
        else if (std::isalpha(c) != 0)
        {
            str_t s;
            for (; std::isdigit(c) != 0 || std::isalpha(c) != 0; c = ReadChar())
                s.push_back(c);
            UnreadChar();
            return Token::Parse(s, PopPos());
        }
        else if (c == '<' || c == '=' || c == '>' || c == '!')
        {
            const auto p = PeekChar();
            if (p != '=')
                return Token::Parse(Char2String(c), PopPos());
            ReadChar();
            return Token::Parse(str_t{c, p}, PopPos());
        }
        else if (c == '/')
        {
            auto p = PeekChar();
            if (p == '/')
            {
                for (; c != '\n'; c = ReadChar())
                    ;
                return Next();
            }
            else if (p == '*')
            {
                ReadChar();
                c = ReadChar();
                for (p = ReadChar(); !(c == '*' && p == '/') && 0 != p; c = p, p = ReadChar())
                    ;
                return Next();
            }
            return Token::Parse(Char2String(c), PopPos());
        }
        else if (Token::IsSign(c))
        {
            return Token::Parse(Char2String(c), PopPos());
        }
        else if (c == '\'')
        {
            auto t = ParseByte();
            c = ReadChar();
            if (c != '\'')
                return Token::Error("invalid byte define", PopPos());
            return t;
        }
        else if (c == '\"')
        {
            str_t s;
            for (c = PeekChar(); '\"' != c && 0 != c; c = PeekChar())
            {
                const auto t = ParseByte();
                if (t.IsError())
                    return Token::Error("invalid string define", PopPos());
                s.push_back(t.GetChar());
            }
            c = ReadChar();
            if (c != '\"')
                return Token::Error("invalid byte define", PopPos());
            return Token(TokenType::STR, s, PopPos());
        }

        return Token::Error("invalid char", PopPos());
    }

    TokenList Tokenizer::All()
    {
        std::vector<Token> tokens;
        for (auto token = Next(); !token.IsNul(); token = Next())
        {
            tokens.emplace_back(token);
            if (token.IsError())
                break;
        }
        return tokens;
    }

    /*
    <integer-literal> ::=
        <decimal-literal>|<hexadecimal-literal>
    <decimal-literal> ::=
        '0'|<nonzero-digit>{<digit>}

    <sign> ::=
        '+'|'-'
    <digit-seq> ::=
        <digit>{<digit>}
    <floating-literal> ::=
            [<digit-seq>]'.'<digit-seq>[<exponent>]
        |<digit-seq>'.'[<exponent>]
        |<digit-seq><exponent>
    <exponent> ::=
        ('e'|'E')[<sign>]<digit-seq>
    */
    Token Tokenizer::ParseDigit()
    {
        Token t;
        size_t cnt = 0;

        const auto base = PeekStr();
        auto ptr = base;
        for (; nullptr != ptr; ++ptr)
        {
            if (std::isdigit(*ptr) == 0)
                break;
        }

        if (*ptr == '.')
        {
            try
            {
                static_assert(sizeof(double) == sizeof(float_t), "float_t size error");
                const auto f = std::stod(base, &cnt);
                t = Token(f, PopPos());
            }
            catch (...)
            {
                t = Token::Error("parse floating literal failed", PopPos());
            }
        }
        else
        {
            try
            {
                static_assert(sizeof(int) == sizeof(int_t), "int_t size error");
                const auto i = std::stoi(base, &cnt, 0);
                if (cnt > 1 && *base == '0' && std::isdigit(*(base + 1)) != 0)
                    t = Token::Error("octal based literal is banned", PopPos());
                else
                    t = Token(i, PopPos());
            }
            catch (...)
            {
                t = Token::Error("parse integer literal failed", PopPos());
            }
        }
        if (t.IsError())
            return t;

        for (size_t i = 0; i < cnt; ++i)
            ReadChar();
        t.SetPosRange(PopPos());

        const auto c = PeekChar();
        if (std::isspace(c) == 0 && c != ';' && c != ',' && c != ')' && c != ':')
        {
            if (t.GetType() == TokenType::FLOAT)
                return Token::Error("invalid floating literal", PopPos());
            else
                return Token::Error("invalid integer literal", PopPos());
        }

        return t;
    }

    Token Tokenizer::ParseByte()
    {
        auto c = ReadChar();
        if (std::isprint(c) == 0)
            return Token::Error("unprintable char", PopPos());

        if (c != '\\')
            return Token(c, PopPos());
        return ParseEscapeSeq();
    }

    /*
    <escape-seq> ::=
        '\\' | "\'" | '\"' | '\n' | '\r' | '\t'
        | '\x'<hexadecimal-digit><hexadecimal-digit>
    <digit> ::=
        '0'|<nonzero-digit>
    <nonzero-digit> ::=
        '1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'
    <hexadecimal-digit> ::=
        <digit>|'a'|'b'|'c'|'d'|'e'|'f'|'A'|'B'|'C'|'D'|'E'|'F'
    */
    Token Tokenizer::ParseEscapeSeq()
    {
        auto c = ReadChar();
        switch (c)
        {
        case '\\': c = '\\'; break;
        case '\'': c = '\''; break;
        case '"': c = '\"'; break;
        case 'n': c = '\n'; break;
        case 'r': c = '\r'; break;
        case 't': c = '\t'; break;
        case 'x':
            {
                const auto a = ReadChar();
                const auto b = ReadChar();
                if (std::isxdigit(a) == 0 || std::isxdigit(b) == 0)
                    return Token::Error("invalid hexadecimal escape sequence", PopPos());
                const auto n = std::stoi(str_t{a, b}, nullptr, 16);
                if (std::isprint(n) == 0)
                    return Token::Error("hexadecimal escape sequence is unprintable", PopPos());
                c = char_t(n);
                break;
            }
        default:
            return Token::Error("invalid escape sequence", PopPos());
        }
        return Token(c, PopPos());
    }

    pos_t Tokenizer::CurPos() const
    {
        return _pos;
    }

    pos_t Tokenizer::PrevPos() const
    {
        if (_pos.first == 0 && _pos.second == 0)
            return _pos;
        if (_pos.second == 0)
            return std::make_pair(_pos.first - 1, _lines[_pos.first - 1].size() - 1);
        else
            return std::make_pair(_pos.first, _pos.second - 1);
    }
    
    pos_t Tokenizer::NextPos() const
    {
        if (IsEOF())
            return _pos;
        if (_pos.second == _lines[_pos.first].size() - 1)
            return std::make_pair(_pos.first + 1, 0);
        else
            return std::make_pair(_pos.first, _pos.second + 1);
    }

    void Tokenizer::PushPos()
    {
        _oldPos = CurPos();
    }

    posrange_t Tokenizer::PopPos()
    {
        return std::make_pair(_oldPos, _pos);
    }

    bool Tokenizer::IsEOF() const
    {
        return _pos.first >= _lines.size();
    }

    const char_t* Tokenizer::PeekStr() const
    {
        if (IsEOF())
            return nullptr;
        return &_lines[_pos.first][_pos.second];
    }

    char_t Tokenizer::PeekChar() const
    {
        if (IsEOF())
            return 0;
        auto c = _lines[_pos.first][_pos.second];
        if (c < 0)
        {
            std::cerr << "non ascii character detected at " << std::to_string(_pos) << std::endl;
            Dump(_pos, std::cerr);
            c = 0;
        }
        return c;
    }

    char_t Tokenizer::ReadChar()
    {
        const auto c = PeekChar();
        _pos = NextPos();
        return c;
    }

    void Tokenizer::UnreadChar()
    {
        _pos = PrevPos();
    }
}