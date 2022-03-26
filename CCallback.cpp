#include "pch.h"
#include "CCallback.h"

template<class T, class... Args>
CCallback<T(Args...)>::CCallback() : callbacks()
{

}

template<class T, class... Args>
CCallback<T(Args...)>::~CCallback()
{
    UnbindAll();
}

template<class T, class... Args>
CCallback<T(Args...)> &CCallback<T(Args...)>::operator+=(CFunction<T, Args...> Y)
{
    callbacks.push_back(Y);
    return *this;
}

template<class T, class... Args>
CCallback<T(Args...)> &CCallback<T(Args...)>::operator-=(CFunction<T, Args...> Y)
{
    callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), Y), callbacks.end());
    return *this;
}

template<class T, class... Args>
void CCallback<T(Args...)>::UnbindAll()
{
    callbacks.clear();
}

template<class T, class... Args>
void CCallback<T(Args...)>::operator()(Args... args)
{
    for (CFunction<T, Args...> func : callbacks)
        (*func)(args...);
}