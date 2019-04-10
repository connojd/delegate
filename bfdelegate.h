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

#include <memory>
#include <functional>
#include <type_traits>

// -----------------------------------------------------------------------------
// static delegate
// -----------------------------------------------------------------------------

template<class Ret, class... Args>
struct static_delegate
{
private:
    using fn_t = Ret(*)(Args...);
    fn_t m_fn;

public:
    template<class R, class... A> explicit
    static_delegate(R(*fn)(A...)) : m_fn{static_cast<fn_t>(fn)}
    {}

    template<class... A> constexpr decltype(auto)
    operator()(A&&... args) const
    {
        return std::invoke(m_fn, args...);
    }
};

// Deduction guides
//
template<class R, class... A>
static_delegate(R(A...)) -> static_delegate<R, A...>;

// -----------------------------------------------------------------------------
// member delegate
// -----------------------------------------------------------------------------

template<class... Args>
struct strip
{
    using T = std::tuple<Args...>;
};

template<class Ret, class... Args>
struct member_delegate
{
private:
    static_assert(sizeof...(Args) >= 1);
    typedef typename std::tuple_element<0, std::tuple<Args...>>::type C;
    typedef Ret(C::*fn_t)();

    C *m_pt;
    fn_t m_fn;

public:
    template<class T, class R, class... A> explicit
    member_delegate(R(T::*fn)(A...), T *obj) :
        m_fn{static_cast<fn_t>(fn)},
        m_pt{static_cast<C *>(obj)}
    {}

    template<class... A> constexpr decltype(auto)
    operator()(A&&... args) const
    {
        return std::invoke((Ret(C::*)(decltype(args)...))m_fn, m_pt, args...);
    }
};

template<class T, class R, class... A>
member_delegate(R(T::*)(A...), T*) -> member_delegate<R, T, A...>;



//template<class Ret, class... Args>
//struct delegate
//{
//private:
//    using stub_t = Ret(*)(Args...);
//    stub_t m_stub;
//    void *m_obj;
//
//    template<size_t n, class... A>
//    invoke(A&&... args) const;
//
////    template<size_t n, class... A>
////    constexpr decltype(auto) call(A&&... args) const
////    {
////        if constexpr (n == 1) {
////            return std::invoke(m_stub, args...);
////        } else if constexpr (n == 2) {
////            return std::invoke(m_stub, m_obj, args...);
////        }
////    }
//
////    using call_pure = template<class... A> call<1, A...>;
////    using call_member = template<class... A> call<2, A...>;
//
//    template<size_t n>
//    struct call {
//        operator()() {
//            if constexpr (n == 1) {
//                std::cout << "hello\n";
//            }
//            if constexpr (n == 2) {
//                std::cout << "hola\n";
//            }
//        }
//    };
//
//    struct call
//
//public:
//    template<class R, class... A> explicit
//    delegate(R(*fn)(A...)) : m_stub{static_cast<stub_t>(fn)}
//    {
//    }
//
////    template<class C, class R, class... A> explicit
////    delegate(R(C::*fn)(A...), C *obj) :
////        m_stub{static_cast<stub_t>(fn)},
////        m_obj{obj}
////    {}
//
//    template<class... A> constexpr decltype(auto)
//    operator()(A&&... args) const
//    {
//        std::invoke(m_stub, args...);
//    }
//};
//
//template<class Ret, class... Args>
//template<size_t n, class... A>
//constexpr decltype(auto)
//delegate<Ret, Args...>::invoke(A... args) const
//{
//    if constexpr (n == 1) {
//        return std::invoke(m_stub, args...);
//    } else if constexpr (n == 2) {
//        return std::invoke(m_stub, m_obj, args...);
//    }
//}
//
//template<class R, class... A> delegate(R(A...)) -> delegate<R, A...>;

// -----------------------------------------------------------------------------
// Delegate Partial Specialization
// -----------------------------------------------------------------------------

/// Delegate
///
/// This delegate class provides the ability to register a call back function
/// that originates from:
/// - A free function
/// - A member function
/// - A lambda function (including capture lists)
///
/// It should be noted that this class attempts to provide the most effcient
/// implementation without the need for malloc / free, or inheritance, at
/// the expense of a very specific syntax requirement.
///
//template <
//    typename RET,
//    typename ...PARAMS
//    >
//class delegate<RET(PARAMS...)>
//{
//    using stub_t = RET(*)(void *, PARAMS...);       ///< Stub function type
//
//public:
//
//    /// Default Constructor
//    ///
//    /// Creates a nullptr delegate. Note that attempts to execute a nullptr
//    /// delegate will result in a crash as we do not perform a check for
//    /// null
//    ///
//    delegate() noexcept = default;
//
//    /// Default Destructor
//    ///
//    ~delegate() noexcept = default;
//
//    /// Is Valid
//    ///
//    /// @return true if valid, false otherwise
//    ///
//    constexpr bool is_valid() const noexcept
//    { return m_stub != nullptr; }
//
//    /// Is Valid
//    ///
//    /// @return true if valid, false otherwise
//    ///
//    constexpr explicit operator bool() const noexcept
//    { return is_valid(); }
//
//    /// Create (Member Function Pointer)
//    ///
//    /// Example usage:
//    /// @code
//    /// test_class t;
//    /// auto d = delegate<int(int)>::create<test_class, &test_class::foo>(&t);
//    /// std::cout << "d: " << d(1) << '\n';
//    /// @endcode
//    ///
//    /// @param obj a pointer to the class who's member function will
//    ///     be executed.
//    /// @return resulting delegate
//    ///
//    template <
//        typename T,
//        RET(T::*FUNC)(PARAMS...),
//        typename = std::enable_if<std::is_class<T>::value>
//        >
//    constexpr static delegate create(T &obj) noexcept
//    { return delegate(std::addressof(obj), member_stub<T, FUNC>); }
//
//    /// Create (Member Function Pointer Class Pointer)
//    ///
//    /// Example usage:
//    /// @code
//    /// test_class t;
//    /// auto d = delegate<int(int)>::create<test_class, &test_class::foo>(&t);
//    /// std::cout << "d: " << d(1) << '\n';
//    /// @endcode
//    ///
//    /// @param obj a pointer to the class who's member function will
//    ///     be executed.
//    /// @return resulting delegate
//    ///
//    template <
//        typename T,
//        RET(T::*FUNC)(PARAMS...),
//        typename = std::enable_if<std::is_class<T>::value>
//        >
//    constexpr static delegate create(T *obj) noexcept
//    { return delegate(obj, member_stub<T, FUNC>); }
//
//    /// Create (Member Function Unique Pointer)
//    ///
//    /// Example usage:
//    /// @code
//    /// auto t = make_unique<test_class>();
//    /// auto d = delegate<int(int)>::create<test_class, &test_class::foo>(t);
//    /// std::cout << "d: " << d(1) << '\n';
//    /// @endcode
//    ///
//    /// @param obj a pointer to the class who's member function will
//    ///     be executed.
//    /// @return resulting delegate
//    ///
//    template <
//        typename T,
//        RET(T::*FUNC)(PARAMS...),
//        typename = std::enable_if<std::is_class<T>::value>
//        >
//    constexpr static delegate create(const std::unique_ptr<T> &obj) noexcept
//    { return delegate(obj.get(), member_stub<T, FUNC>); }
//
//    /// Create (Const Member Function Pointer)
//    ///
//    /// Example usage:
//    /// @code
//    /// test_class t;
//    /// auto d = delegate<int(int)>::create<test_class, &test_class::foo>(&t);
//    /// std::cout << "d: " << d(1) << '\n';
//    /// @endcode
//    ///
//    /// @note this function has to have _const appended to it's name because
//    ///     MSVC doesn't understand the template overload and fails to
//    ///     compile.
//    ///
//    /// @param obj a pointer to the class who's member function will
//    ///     be executed.
//    /// @return resulting delegate
//    ///
//    template <
//        typename T,
//        RET(T::*FUNC)(PARAMS...) const,
//        typename = std::enable_if<std::is_class<T>::value>
//        >
//    constexpr static delegate create_const(const T &obj) noexcept
//    { return delegate(const_cast<T *>(std::addressof(obj)), const_member_stub<T, FUNC>); }
//
//    /// Create (Function Pointer)
//    ///
//    /// Example usage:
//    /// @code
//    /// auto d = delegate<int(int)>::create<&foo>();
//    /// std::cout << "d: " << d(1) << '\n';
//    /// @endcode
//    ///
//    /// @return resulting delegate
//    ///
//    template <RET(FUNC)(PARAMS...)>
//    constexpr static delegate create() noexcept
//    { return delegate(nullptr, function_stub<FUNC>); }
//
//    /// Create (Lambda Pointer)
//    ///
//    /// Example usage:
//    /// @code
//    /// test_class t;
//    /// auto d = delegate<int(int)>::create([](int val) -> int { return val; });
//    /// std::cout << "d: " << d(1) << '\n';
//    /// @endcode
//    ///
//    /// @param ptr a pointer to the lambda function that will
//    ///     be executed.
//    /// @return resulting delegate
//    ///
//    template <typename LAMBDA>
//    constexpr static delegate create(const LAMBDA &ptr) noexcept
//    { return delegate(reinterpret_cast<void *>(const_cast<LAMBDA *>(std::addressof(ptr))), lambda_stub<LAMBDA>); }
//
//    /// Operator ()
//    ///
//    /// @param params the parameters to pass to the delegate function.
//    /// @return the result of executing the delegate function
//    ///
//    constexpr RET operator()(PARAMS... params) const
//    { return (*m_stub)(m_obj, std::forward<PARAMS>(params)...); }
//
//private:
//
//    /// @cond
//
//    // Note:
//    //
//    // The only public constructor is the default constructor. To create a
//    // delegate, use the delegate::create function.
//    //
//
//    constexpr explicit delegate(void *obj, stub_t stub) noexcept :
//        m_obj(obj),
//        m_stub(stub)
//    { }
//
//    /// @endcond
//
//private:
//
//    /// @cond
//
//    template <
//        typename T,
//        RET(T::*FUNC)(PARAMS...),
//        typename = std::enable_if<std::is_class<T>::value>
//        >
//    constexpr static RET member_stub(void *obj, PARAMS... params)
//    { return (static_cast<T *>(obj)->*FUNC)(std::forward<PARAMS>(params)...); }
//
//    template <
//        typename T,
//        RET(T::*FUNC)(PARAMS...) const,
//        typename = std::enable_if<std::is_class<T>::value>
//        >
//    constexpr static RET const_member_stub(void *obj, PARAMS... params)
//    { return (static_cast<T *>(obj)->*FUNC)(std::forward<PARAMS>(params)...); }
//
//    template <RET(*FUNC)(PARAMS...)>
//    constexpr static RET function_stub(void *, PARAMS... params)
//    { return (FUNC)(std::forward<PARAMS>(params)...); }
//
//    template <typename LAMBDA>
//    constexpr static RET lambda_stub(void *ptr, PARAMS... params)
//    { return (static_cast<LAMBDA *>(ptr)->operator())(std::forward<PARAMS>(params)...); }
//
//    /// @endcond
//
//private:
//
//    void *m_obj{nullptr};       ///< Object containing member function
//    stub_t m_stub{nullptr};     ///< Stub that executes the delegate function
//
//public:
//
//    /// @cond
//
//    delegate(const delegate &other) noexcept = default;
//    delegate &operator=(const delegate &other) noexcept = default;
//
//    delegate(delegate &&other) noexcept = default;
//    delegate &operator=(delegate &&other) noexcept = default;
//
//    /// @endcond
//};

#endif
