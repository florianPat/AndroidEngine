#pragma once

struct GenericDelegateClass {};

template <typename T>
class Delegate;

template <typename R, typename ... Args>
class Delegate<R(Args...)>
{
    typedef R(*MemFn)(void*, Args...);

    void* thiz = nullptr;
    MemFn memFn = nullptr;

    Delegate(void* thiz, MemFn memFn) : thiz(thiz), memFn(memFn) {}

    template <R(*functionPtr)(Args...)>
    static R functionStub(void*, Args... args)
    {
        return functionPtr(std::forward<Args>(args)...);
    }

    template <typename C, R(C::*methodPtr)(Args...)>
    static R methodStub(void* thiz, Args... args)
    {
        return (((C*)thiz)->*methodPtr)(std::forward<Args>(args)...);
    }
public:
    template <class C, R(C::*methodPtr)(Args...)>
    static Delegate from(C* thiz)
    {
        return Delegate<R(Args...)>(thiz, &methodStub<C, methodPtr>);
    }

    template <R(*functionPtr)(Args...)>
    static Delegate from()
    {
        return Delegate(nullptr, &functionStub<functionPtr>);
    }

    R operator()(Args... args) const
    {
        return memFn(thiz, std::forward<Args>(args)...);
    }
};