#ifndef __INTEGER_H__
#define __INTEGER_H__
// integer.h
template <typename SignedInt>
class Integer {
    SignedInt value;

public:
    Integer(SignedInt i);
    void doSomething();
};
typedef Integer<int> Integer32;
typedef Integer<long long> Integer64;

#endif  // __INTEGER_H__