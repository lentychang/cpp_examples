# Template explicit instantiation
```c++
template<typename T>
struct A{}
using A_int = struct A<int>; // This is put in header [1]

template struct A<int>; // This is called explicit instantiation and is put in the definition file. 
```

# Purpose of instantiation
You create template, and want restrict your user to use only certain types.
Three files: `integer.h`,`integer.cpp`, `main.cpp`  
Since usually `main.cpp` includes only header, library end user cannot know he/she is not allowed to use types other than the specific types specified in 



```c++
// integer.h
template<typename SignedInt>
class Integer
{
    SignedInt value;
public:
    Integer(SignedInt i);
    void doSomething();
};
typedef Integer<int> Integer32;
typedef Integer<long long> Integer64;
```
```c++
// integer.cpp
#include "integer.h"

template<typename SignedInt>
class Integer
{
    SignedInt value;
public:
    Foo(SignedInt i):value{i}{};
    void doSomething();
};
template class Integer<int>;
template class Integer<long long>;
```
```c++
#include "integer.h"
int main(){
    Integer<int> int32{123};
    Integer<long long> int64{123};
    return 0;
}
```


## Ref:
[Stackoverflow Explicit instanitiation](https://stackoverflow.com/questions/2351148/explicit-template-instantiation-when-is-it-used) See the answer from Martin York

[Reduce Comile time for template](https://arne-mertz.de/2019/02/extern-template-reduce-compile-times/) Read it carefully!

[Reduce Comile time for template](https://stackoverflow.com/questions/8130602/using-extern-template-c11)