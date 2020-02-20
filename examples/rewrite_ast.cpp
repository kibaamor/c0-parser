#include <tokenizer.h>
#include <analyser.h>
#include <dump_visitor.h>
#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
    if (1 == argc)
    {
        std::cout << "usage: " << argv[0] << " file" << std::endl;
        return 0;
    }

    std::ifstream ifs(argv[1]);
    if (!ifs)
    {
        std::cerr << "error open: " << argv[1] << std::endl;
        return -1;
    }

    c0::Tokenizer tzer(ifs);
    const auto tokens = tzer.All();
    if (!tokens.empty() && tokens.back().IsError())
    {
        std::cerr << std::to_string(tokens.back()) << std::endl;
        tzer.Dump(tokens.back().GetPosRange().first, std::cerr);
        return -2;
    }

    c0::Analyser ayer(tokens);
    c0::AnalyseError err;

    auto ast = ayer.Analyse(err);
    if (err)
    {
        err.FixSource(tzer.GetLines());
        std::cerr << std::to_string(err) << std::endl;
    }
    else
    {
        if (nullptr != ast)
        {
            c0::DumpVisitor visitor(std::cout);
            ast->Accept(visitor);
        }
    }

    return 0;
}
