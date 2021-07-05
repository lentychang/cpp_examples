#include <string>
#include <iostream>
#include <regex>

int main()
{
    std::string s{"   192  .168.  1   .1   "};
    std::smatch sm{};

    std::regex rgx{"([0-9]{1,3})"};

    std::string::const_iterator searchStart( s.cbegin() );
    while (std::regex_search(searchStart,s.cend(), sm, rgx))
    {
        std::cout << sm[1].str() << std::endl;
        searchStart = sm.suffix().first;
    }
    return 0;
}