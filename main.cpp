#include <iostream>
#include <map>
#include <memory>
#include <cassert>
#include <iterator>
#include <stdexcept>

template <typename T, size_t N>
class FixedAllocator {
    public:
        using value_type = T;
    FixedAllocator(): allocated(0) {
        data = reinterpret_cast<T*>(::operator new(sizeof(T) * N));
    }
template <typename U>
FixedAllocator(const FixedAllocator<U, N>&) noexcept {}

template <typename U>
struct rebind {
    using other = FixedAllocator<U,N>;
}; 

~FixedAllocator() {
    for(std::size_t i = 0; i < allocated; i++){
        data[i].~T();
    }
    ::operator delete(data);
}

T* allocate(std::size_t n) {
    if (allocated + n > N) 
        throw std::bad_alloc();
    T* ptr = data + allocated;
    allocated += n;
    return ptr;
}

void deallocate(T*, std::size_t) noexcept {}

private:
    T* data;
    std::size_t allocated;

    template <typename U, std::size_t M>
    friend class FixedAllocator;
};

// comparisonn operators
template <typename T1, typename T2, std::size_t N>
bool operator==(const FixedAllocator<T1, N>&, const FixedAllocator<T2, N>&) {return true;}

template <typename T1, typename T2, std::size_t N>
bool operator!=(const FixedAllocator<T1, N>&, const FixedAllocator<T2, N>&) {return false;}

// factorial func
int factorial(int n) {
    return (n <= 1 ) ? 1: n * factorial(n - 1);
}

// my container
template <typename T, typename Alloc = std::allocator<T>>
class MyContainer {
    public: 
        using allocator_type = Alloc;
        using value_type = T;
        using pointer = typename std::allocator_traits<Alloc>::pointer;

        MyContainer(): size_(0) {}
        MyContainer(const Alloc& alloc) : allocator(alloc), size_(0) {}

        ~MyContainer() {
            for(std::size_t i = 0; i < size_; i++)
                std::allocator_traits<Alloc>::destroy(allocator, data + i);
            allocator.deallocate(data, capacity);
        }

        void add(const T& value) {
            if (size_ == capacity) {
                if(capacity == 0)
                    capacity = 10;
                pointer new_data = allocator.allocate(capacity);
                for(std::size_t i = 0; i < size_; i++)
                    std::allocator_traits<Alloc>::construct(allocator, new_data + i, data[i]);
                if (data) allocator.deallocate(data, capacity);
                    data = new_data;
            }
            std::allocator_traits<Alloc>::construct(allocator, data + size_, value);
            ++size_;
        }

        void print() const {
            for(std::size_t i = 0; i < size_; i++)
                std::cout << data[i] << " ";
            std::cout << std::endl;
        }

        std::size_t size() const { return size_; }
        bool empty() const { return size_ == 0; }

    private:
        Alloc allocator;
        pointer data = nullptr;
        std::size_t size_;
        std::size_t capacity = 0;
};

int main()
{
    // default map
    std::map<int, int> default_map;
    for(int i = 0; i < 10; ++i)
        default_map[i] = factorial(i);

    for (const auto& pair : default_map) {
    int k = pair.first;
    int v = pair.second;
    std::cout << k << " " << v << std::endl;
}
    
    // default map with my allocator
    std::map<int, int, std::less<>,  FixedAllocator<std::pair<const int, int>, 10>> custom_map;
    for(int i = 0; i < 10; ++i)
        custom_map[i] = factorial(i);
    
    for (const auto& pair : custom_map) {
        int k = pair.first;
        int v = pair.second;
        std::cout << k << " " << v << std::endl;
    }

    // MyContainer with default allocator
    MyContainer<int> myContainer_default;
    for(int i = 0; i < 10; ++i)
        myContainer_default.add(i);
    myContainer_default.print();

    // MyContainer with custom allocator
    MyContainer<int, FixedAllocator<int, 10>> myContainer_custom;
    for(int i = 0; i < 10; ++i)
        myContainer_custom.add(i);
    myContainer_custom.print();

    return 0;
    
}