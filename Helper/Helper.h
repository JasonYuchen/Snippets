#pragma once
#include <type_traits>
#include <mutex>
#include <memory>
#include <functional>
#include <cassert>
template<typename E>
constexpr auto underlyingType_FromEnum(E enumerator) noexcept {
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

template<typename T>
class CopyOnWrite {
    static_assert(!std::is_reference<T>::value, "<FAILED> template type of CopyOnWrite should be a value type");
public:
    explicit CopyOnWrite(const T &data) : data_(std::make_shared<T>(data)) {}
    explicit CopyOnWrite(T &&data) : data_(std::make_shared<T>(data)) {}
    explicit CopyOnWrite(std::shared_ptr<T> data) : data_(data) {}

    void read(std::function<void(const T &)> func) { 
        std::shared_ptr<T> data;
        {
            std::lock_guard<std::mutex> guard(lock_);
            data = data_;
            assert(!data_.unique());
        }
        func(*data);
    }

    void modify(std::function<void(T &)> func) {
        std::lock_guard<std::mutex> guard(lock_);
        if (!data_.unique()) {
            data_.reset(new T(*data_));
        }
        assert(data_.unique());
        func(*data_);
    }

private:
    std::mutex lock_;
    std::shared_ptr<T> data_;
};