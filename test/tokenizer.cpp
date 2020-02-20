#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <tokenizer.h>
#include <sstream>

TEST_SUITE_BEGIN("tokenizer");
using namespace c0;

TEST_CASE("int parse")
{
    std::string s = R"(
0
1
0x7fffffff
0X7fffffff
)";
    std::istringstream is(s);
    Tokenizer tzer(is);
    const auto tokens = tzer.All();

    CHECK((!tokens.empty() && !tokens.back().IsError()));
    CHECK(tokens.size() == 4);
    for (const auto& t : tokens)
    {
        std::cout << std::to_string(t) << std::endl;
        CHECK(t.GetType() == TokenType::INT);
    }

    CHECK(tokens[0].GetInt() == 0);
    CHECK(tokens[1].GetInt() == 1);
    CHECK(tokens[2].GetInt() == 0x7fffffff);
    CHECK(tokens[3].GetInt() == 0x7fffffff);
}

TEST_CASE("float parse")
{
    std::string s = R"(
12.
.34
12.34

12.e1
12.e+1
12.e-1

.34e1
.34e+1
.34e-1

12.34e1
12.34e+1
12.34e-1
)";
    std::istringstream is(s);
    Tokenizer tzer(is);
    const auto tokens = tzer.All();

    CHECK(tokens.size() == 12);
    for (const auto& t : tokens)
    {
        std::cout << std::to_string(t) << std::endl;
        CHECK(t.GetType() == TokenType::FLOAT);
    }
}

TEST_SUITE_END();
