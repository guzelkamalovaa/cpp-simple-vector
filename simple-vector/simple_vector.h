#pragma once
#include "array_ptr.h"

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
        : size_(size), capacity_(size), data_(size ? AllocateAndDefaultInitialize(size) : nullptr) {}

    SimpleVector(size_t size, const Type& value)
        : size_(size), capacity_(size), data_(size ? AllocateAndFill(size, value) : nullptr) {}

    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()), capacity_(init.size()), data_(init.size() ? AllocateAndCopy(init.begin(), init.end()) : nullptr) {}

    explicit SimpleVector(ReserveType r)
        : size_(0), capacity_(r.capacity), data_(r.capacity ? Allocate(r.capacity) : nullptr) {}

    SimpleVector(const SimpleVector& other)
        : size_(other.size_), capacity_(other.capacity_), data_(other.capacity_ ? AllocateAndCopy(other.begin(), other.end()) : nullptr) {}

    SimpleVector(SimpleVector&& other) noexcept
        : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
        other.size_ = 0;
        other.capacity_ = 0;
        other.data_ = nullptr;
    }

    ~SimpleVector() {
        DestroyElements();
        operator delete(data_);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type> tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
    if (this != &rhs) {
        SimpleVector tmp(std::move(rhs)); 
        swap(tmp);                        
    }
    return *this;
}

    void PushBack(const Type& item) {
        EmplaceBack(item);
    }

    void PushBack(Type&& item) {
        EmplaceBack(std::move(item));
    }

    template <typename... Args>
    Type& EmplaceBack(Args&&... args) {
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (data_ + size_) Type(std::forward<Args>(args)...);
        ++size_;
        return data_[size_ - 1];
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        return Emplace(pos, value);
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        return Emplace(pos, std::move(value));
    }

    template <typename... Args>
    Iterator Emplace(ConstIterator pos, Args&&... args) {
        size_t index = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            Type* new_data = Allocate(new_capacity);

            for (size_t i = 0; i < index; ++i) {
                new (new_data + i) Type(std::move(data_[i]));
                data_[i].~Type();
            }

            new (new_data + index) Type(std::forward<Args>(args)...);

            for (size_t i = index; i < size_; ++i) {
                new (new_data + i + 1) Type(std::move(data_[i]));
                data_[i].~Type();
            }

            operator delete(data_);

            data_ = new_data;
            capacity_ = new_capacity;
        } else {
            if (index < size_) {
                std::move_backward(data_ + index, data_ + size_, data_ + size_ + 1);
            }
            new (data_ + index) Type(std::forward<Args>(args)...);
        }
        ++size_;
        return data_ + index;
    }

    void PopBack() noexcept {
        assert(size_ > 0 && "PopBack called on empty vector");
        data_[size_ - 1].~Type();
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        size_t index = pos - begin();
        assert(index < size_ && "Erase position out of range");
        data_[index].~Type();
        for (size_t i = index; i < size_ - 1; ++i) {
            new (data_ + i) Type(std::move(data_[i + 1]));
            data_[i + 1].~Type();
        }
        --size_;
        return data_ + index;
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(data_, other.data_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            Type* new_data = Allocate(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                new (new_data + i) Type(std::move(data_[i]));
                data_[i].~Type();
            }
            operator delete(data_);
            data_ = new_data;
            capacity_ = new_capacity;
        }
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return data_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    void Clear() noexcept {
        DestroyElements();
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            Reserve(new_capacity);
        }
        if (new_size > size_) {
            for (size_t i = size_; i < new_size; ++i) {
                new (data_ + i) Type();
            }
        } else {
            for (size_t i = new_size; i < size_; ++i) {
                data_[i].~Type();
            }
        }
        size_ = new_size;
    }

    Iterator begin() noexcept {
        return data_;
    }

    Iterator end() noexcept {
        return data_ + size_;
    }

    ConstIterator begin() const noexcept {
        return data_;
    }

    ConstIterator end() const noexcept {
        return data_ + size_;
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    Type* data_ = nullptr;

    Type* Allocate(size_t n) {
        return static_cast<Type*>(operator new(n * sizeof(Type)));
    }

    Type* AllocateAndDefaultInitialize(size_t n) {
        Type* new_data = Allocate(n);
        for (size_t i = 0; i < n; ++i) {
            new (new_data + i) Type();
        }
        return new_data;
    }

    Type* AllocateAndFill(size_t n, const Type& value) {
        Type* new_data = Allocate(n);
        for (size_t i = 0; i < n; ++i) {
            new (new_data + i) Type(value);
        }
        return new_data;
    }

    Type* AllocateAndCopy(ConstIterator first, ConstIterator last) {
        size_t n = last - first;
        Type* new_data = Allocate(n);
        size_t i = 0;
        try {
            for (; first != last; ++first, ++i) {
                new (new_data + i) Type(*first);
            }
        } catch (...) {
            for (size_t j = 0; j < i; ++j) {
                new_data[j].~Type();
            }
            operator delete(new_data);
            throw;
        }
        return new_data;
    }

    void DestroyElements() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~Type();
        }
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

template <typename Type>
inline void swap(SimpleVector<Type>& lhs, SimpleVector<Type>& rhs) noexcept {
    lhs.swap(rhs);
}