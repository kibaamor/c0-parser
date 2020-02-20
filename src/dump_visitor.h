#pragma once
#include "ast.h"

namespace c0
{
    class DumpVisitor : public ASTVisitor
    {
    public:
        DumpVisitor(std::ostream& stream) : _stream(stream) {}

        bool BegVisit(const AST& ast) override;
        bool EndVisit(const AST& ast) override;

    private:
        str_t GetIndent() const;
        std::ostream& Out() const;

    private:
        std::ostream& _stream;
        int _indent = 0;
    };
}