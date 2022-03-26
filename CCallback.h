#pragma once

#define CFunctionFactory(f, ...) ((CFunction<__VA_ARGS__>)&f)

template<class T, class... Args>
using CFunction = T(*)(Args...);

template<class T>
struct CCallback;

template<class T, class... Args>
struct CCallback<T(Args...)>
{
    CCallback();
    ~CCallback();

    std::vector<CFunction<T, Args...>> callbacks;

    CCallback<T(Args...)> &operator+=(CFunction<T, Args...> Y);
    CCallback<T(Args...)> &operator-=(CFunction<T, Args...> Y);

    void UnbindAll();
    void operator()(Args... args);
};

