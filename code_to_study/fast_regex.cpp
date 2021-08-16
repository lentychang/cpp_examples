#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>
#include <string>
#include <string_view>
// can also try c++ regex

using namespace std;
using namespace std::experimental;
using namespace boost;

string_view stringfier(const cregex_token_iterator::value_type &match) {
    return {match.first, static_cast<size_t>(match.length())};
}

using string_view_iterator =
    transform_iterator<decltype(&stringfier), cregex_token_iterator>;

iterator_range<string_view_iterator> split(string_view s, const regex &r) {
    return {
        string_view_iterator(
            cregex_token_iterator(s.begin(), s.end(), r, -1),
            stringfier),
        string_view_iterator()};
}

int main() {
    const regex r(" +");
    for (size_t i = 0; i < 1000000; ++i) {
        split("a b c", r);
    }
}

// if copy
// int main() {
//     const regex r(" +");
//     vector<string_view> v;
//     v.reserve(10);
//     for (size_t i = 0; i < 1000000; ++i) {
//         copy(split("a b c", r), back_inserter(v));
//         v.clear();
//     }
// }