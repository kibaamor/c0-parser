#include <tokenizer.h>
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

    for (const auto& token : tokens)
        std::cout << std::to_string(token) << std::endl;

    return 0;
}