// integer.cpp
#include "integer.h"

template <typename SignedInt>
Integer<SignedInt>::Integer(SignedInt i) : value{i} {}

template <typename SignedInt>
void Integer<SignedInt>::doSomething() {}

template class Integer<int>;
template class Integer<long long>;
