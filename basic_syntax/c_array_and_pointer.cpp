#include <algorithm>
#include <array>
#include <iostream>
#include <span>

void func_pass_arr_as_ptr(int* int_ptr, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        std::cout << int_ptr[i] << " ";
        // int_ptr[i] --> *(int_ptr+i)
    }
    std::cout << '\n';
}

void func_pass_arr_as_arr(int arr[], size_t size) {
    for (size_t i = 0; i < size; ++i) {
        // pointer to array have to be access in this way
        std::cout << arr[i] << " ";
    }
    std::cout << '\n';
}

void func_modern_cpp(std::span<int, 10> arr_of_ptr) {
    std::ranges::for_each(arr_of_ptr, [](const int& element) { std::cout << element << " "; });
    std::cout << '\n';
}

int main() {
    // ======= Declaration of array ========
    // (1) Stack array T var_name[size]{...};
    // (2) Heap array T* var_name = new T[size]{...};
    //     Because var_name is a pointer, it can be rebound to large size heap memory.
    //     As a result, it can be used to implement dynamic array like std::vector
    // (3) modern cpp array std::array<T,szie> var_name{...};
    //     User can define their own allocator to decide to allocate on stack or heap
    // (4) modern cpp dynamic array std::vector<T> var_name{...};

    // (1) Stack Array - C-Array
    int c_array_stack[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // (2)Heap Array - C-Array
    //    Array of int, this is dynamic array
    //    c_array_heap can be taken as a pointer to int, the first int in the int array
    int* c_array_heap = new int[]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};  // a array contains 10 int on the heap

    // Array of pointer
    int* c_array_of_pointer[] = {new int(1), new int(1), new int(1)};  // This means arr of 10 int*

    // ----------------------------------------------------------
    // ======= C-Array return type and function argument ========
    // The C++ rule for the return type and value of new T is:

    // If T is not an array type, the return type is T *, and the returned value is a pointer to the dynamically allocated object of type T.
    // If T is an array of type U, the return type is U *, and the returned value is a pointer to the first element (whose type is U) of the dynamically allocated array of type T.

    func_pass_arr_as_ptr(c_array_stack, 10);
    func_pass_arr_as_ptr(c_array_heap, 10);

    func_pass_arr_as_arr(c_array_stack, 10);
    func_pass_arr_as_arr(c_array_heap, 10);

    // func and func2 shows that function argument T var[] and T*var is the same!

    // === Modern-cpp accessing C-array with std::span ===
    func_modern_cpp(std::span<int, 10>{c_array_heap, 10});
    func_modern_cpp(std::span<int, 10>{c_array_stack, 10});

    return 0;
}