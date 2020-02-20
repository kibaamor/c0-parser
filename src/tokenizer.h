#pragma once
#include "token.h"
#include <iosfwd>
#include <vector>
#include <cstddef>

namespace c0
{
    using TokenList = std::vector<Token>;

    class Tokenizer
    {
    public:
        Tokenizer(std::istream& stream);

        void Dump(std::ostream& stream) const;
        void Dump(const pos_t& pos, std::ostream& stream) const;

        Token Next();
        TokenList All();

        const std::vector<std::string>& GetLines() const { return _lines; }

    private:
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
        Token ParseDigit();
        Token ParseByte();
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
        Token ParseEscapeSeq();
        pos_t CurPos() const;
        pos_t PrevPos() const;
        pos_t NextPos() const;
        void PushPos();
        posrange_t PopPos();
        bool IsEOF() const;
        const char_t* PeekStr() const;
        char_t PeekChar() const;
        char_t ReadChar();
        void UnreadChar();

    private:
        std::vector<std::string> _lines;
        pos_t _pos;
        pos_t _oldPos;
    };
}