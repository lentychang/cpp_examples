class String {};
template <class T>
class Array { /*...*/
};
template <class T>
void sort(Array<T>& v) { /*...*/

}  // primary template

// Sepcialized template has to be defined before impicit instantiation
// [For beginner]
// If templated argument is used as an option to select functionality/method of base template
// It is not recommanded, because then you have to implement every function that you select from base!
// You still insisted tu use template argument as an option.
// It is recommand create an other new_Base template that you old base template, and create specialized template for new_base template

/**
 * @brief  This is an explicit specialization of function sort(Array<T>& v). It must be defined before it is called/used which will result implicit instanciation.
 * @tparam  
 */
template <>                             // Success: explicit specialization of sort(Array<String>)
void sort<String>(Array<String>& v){};  // before implicit instantiation

void f(Array<String>& v) {
    sort(v);  // implicitly instantiates sort(Array<String>&),
}  // using the primary template for sort()

/**
 * @brief thestsgdg
 * 
 * 
 */
void test() {
}
// ERROR: explicit specialization of sort(Array<String>)
// template <>
// void sort<String>(Array<String>& v){};  // after implicit instantiation

// template <typename T>
// void baseF(T t) { ... }

// template <typename T>
// void F(T t) { baseF<T>(t); }

// template <>
// void F<int>(int t) { baseF<int>(t); }

int main() {
    return 0;
}