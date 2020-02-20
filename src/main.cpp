#if 0
#include "tokenizer.h"
#include "analyser.h"
#include "dump_visitor.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char** argv)
{
    using namespace c0;

    std::string s = R"(

int main() {
	char a = 0;
while (1)
    switch ('a')
    {
    case 'a':
        {
            print(a);
            break;
        }
    case 'b':
        {
            print(a);
            break;
        } 
    default:
        {
            print("ok");
            break;
        }
    }
	return 0;
}
)";

    std::istringstream is(s);
    Tokenizer tzer(is);
    auto tokens = tzer.All();
    if (!tokens.empty() && tokens.back().IsError())
    {
        std::cerr << std::to_string(tokens.back()) << std::endl;
        tzer.Dump(tokens.back().GetPosRange().first, std::cerr);
        return 0;
    }
#if 1
    for (const auto& token : tokens)
        std::cout << std::to_string(token) << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;
#endif

    Analyser ayer(tokens);
    AnalyseError err;

    auto file = ayer.Analyse(err);
    if (err)
    {
        err.FixSource(tzer.GetLines());
        std::cerr << std::to_string(err) << std::endl;
    }
    else
    {
        if (nullptr != file)
        {
            DumpVisitor visitor(std::cout);
            file->Accept(visitor);
            //std::cout << std::to_string(file) << std::endl;
        }
    }
    file = nullptr;

    if (AST::GetInstanceCount() != 0)
    {
        std::cout << "AST Instance count:" << AST::GetInstanceCount() << std::endl;
        for (const auto& kv : AST::GetInstanceMap())
        {
            if (kv.second == 0)
                continue;
            std::cout << std::to_string(kv.first) << " " << kv.second << std::endl;
        }
    }

    return 0;
}
#endif