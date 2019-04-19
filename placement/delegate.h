//
// Copyright (C) 2019 Assured Information Security, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

///
/// @file bfdelegate.h
///

#ifndef BFDELEGATE_H
#define BFDELEGATE_H

#include <array>
#include <cstdint>
#include <functional>
#include <type_traits>

/// state
///
/// This stores the non-argument state needed by the delegate such
/// as lambdas and object addresses.
///
template<size_t size = 24, size_t align = 32>
class state
{
    alignas(align) std::array<uint8_t, size> m_buf{};
};

using state_t = state<>;

template<class Ret, class... Args>
using call_t = Ret(*)(const state_t&, Args&&...);

/// state helpers
///
/// These functions are lifted from Ben Diamand's implementation at
/// https://github.com/bdiamand/Delegate/blob/master/delegate.h
///
/// They are used to reinterpret a piece of memory as a functor type F
///
template<class F>
static constexpr bool can_emplace()
{
    return (sizeof(F) <= sizeof(state_t)) &&
           (alignof(state_t) % alignof(F) == 0);
}

template<class F>
static F &get_state(const state_t &state)
{ return (F &)(state); }

template<class F>
static void copy_state(state_t &state, const F &fn)
{
    static_assert(can_emplace<F>());
    new (&get_state<F>(state)) F(fn);
}

template<class F>
static void move_state(state_t &state, F &&src)
{
    static_assert(can_emplace<F>());
    new (&get_state<F>(state)) F(src);
}

template<class F, class Ret, class... Args>
static Ret call(const state_t &state, Args&&... args)
{
    static_assert(std::is_invocable_r_v<Ret, F, Args...>);
    return get_state<F>(state)(std::forward<Args>(args)...);
}

/// vtable
///
/// Each delegate has a vtable that contains functions to copy,
/// move, and destroy a given type. It is used to implement
/// the copy/move ctor/assignment ops of the delegate.
///
class vtable {
public:
    void (&copy)(state_t &lhs, const state_t &rhs);
    void (&move)(state_t &lhs, state_t &&rhs);
    void (&destroy)(state_t &state);

    template<class F>
    static const vtable &init() noexcept
    {
        static const vtable self = {
            .copy = s_copy<F>,
            .move = s_move<F>,
            .destroy = s_destroy<F>
        };

        return self;
    }

private:
    template<
        class F,
        typename std::enable_if_t<std::is_copy_constructible_v<F>>* = nullptr
    >
    static void s_copy(state_t &lhs, const state_t &rhs) noexcept
    { copy_state<F>(lhs, get_state<F>(rhs)); }

    template<
        class F,
        typename std::enable_if_t<std::is_move_constructible_v<F>>* = nullptr
    >
    static void s_move(state_t &lhs, state_t &&rhs) noexcept
    { move_state<F>(lhs, std::move(get_state<F>(rhs))); }

    template<
        class F,
        typename std::enable_if_t<std::is_destructible_v<F>>* = nullptr
    >
    static void s_destroy(state_t &state) noexcept
    { get_state<F>(state).~F(); }
};

/// delegate
///
/// Wraps either a raw function pointer or a pointer-to-member-function
/// and pointer-to-object into an invocable type with a common signature.
///
/// The constructor arguments provide the internal state needed to later
/// invoke the function with the Args... arguments. Only normal function
/// pointers and member function pointers are supported, but lambdas could
/// also be used provided their capture list is at most 16 bytes.
///

template<class Ret, class... Args>
class delegate
{
public:
    /// Raw function pointer
    ///
    delegate(Ret(*fn)(Args...))
    {
        m_call = &call<decltype(fn), Ret, Args...>;
        m_vtbl = &vtable::init<decltype(fn)>();
        copy_state(m_state, fn);
    }

    /// Non-const memfn, non-const object
    ///
    template<class C>
    delegate(Ret(C::*memfn)(Args...), C *obj)
    {
        auto fn = [memfn, obj](Args&&... args) -> decltype(auto)
        { return std::invoke(memfn, obj, args...); };

        m_call = &call<decltype(fn), Ret, Args...>;
        m_vtbl = &vtable::init<decltype(fn)>();
        copy_state(m_state, fn);
    }

    /// Const memfn, non-const object
    ///
    template<class C, typename = std::enable_if<std::is_class_v<C>>>
    delegate(Ret(C::*memfn)(Args...) const, C *obj)
    {
        auto fn = [memfn, obj](Args&&... args) -> decltype(auto)
        { return std::invoke(memfn, obj, args...); };

        m_call = &call<decltype(fn), Ret, Args...>;
        m_vtbl = &vtable::init<decltype(fn)>();
        copy_state(m_state, fn);
    }

    /// Const memfn, const object
    ///
    template<class C, typename = std::enable_if<std::is_class_v<C>>>
    delegate(Ret(C::*memfn)(Args...) const, const C *obj)
    {
        auto fn = [memfn, obj](Args&&... args) -> decltype(auto)
        { return std::invoke(memfn, obj, args...); };

        m_call = &call<decltype(fn), Ret, Args...>;
        m_vtbl = &vtable::init<decltype(fn)>();
        copy_state(m_state, fn);
    }

    /// Copy constructor
    ///
    delegate(const delegate &other) :
        m_call{other.m_call},
        m_vtbl{other.m_vtbl}
    { m_vtbl->copy(m_state, other.m_state); }

    /// Move constructor
    ///
    delegate(delegate &&other) :
        m_call{other.m_call},
        m_vtbl{other.m_vtbl}
    { m_vtbl->move(m_state, std::move(other.m_state)); }

    /// Copy assignment
    ///
    delegate &operator=(const delegate &other)
    {
        m_call = other.m_call;
        m_vtbl = other.m_vtbl;
        m_vtbl->copy(m_state, other.m_state);
    }

    /// Move assignment
    ///
    delegate &operator=(delegate &&other)
    {
        m_call = other.m_call;
        m_vtbl = other.m_vtbl;
        m_vtbl->move(m_state, std::move(other.m_state));
    }

    /// Destructor
    ///
   ~delegate()
   { m_vtbl->destroy(m_state); }

    /// Call operator
    ///
    Ret operator()(Args&&... args) const
    { return m_call(m_state, std::forward<Args>(args)...); }

private:
    state_t m_state;
    call_t<Ret, Args...> m_call;
    const vtable *m_vtbl;
};

/// Class deduction guides

template<class R, class... A>
delegate(R(A...)) -> delegate<R, A...>;

template<class C, class R, class... A>
delegate(R(C::*)(A...), C*) -> delegate<R, A...>;

template<class C, class R, class... A>
delegate(R(C::*)(A...) const, C*) -> delegate<R, A...>;

template<class C, class R, class... A>
delegate(R(C::*)(A...) const, const C*) -> delegate<R, A...>;

#endif
