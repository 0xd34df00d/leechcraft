#include "typelist.h"

namespace TypeLists
{
// ========== Length ==========
    template<>
    class Length<NullType>
    {
    public:
        enum { Value = 0 };
    };

    template<typename T, typename U>
    class Length<TypeList<T, U> >
    {
    public:
        enum { Value = Length<U>::Value + 1 };
    };
// ========== TypeAt ==========
    template<typename T, typename U>
    class TypeAt<TypeList<T, U>, 0>
    {
    public:
        typedef T Result;
    };

    template<typename T, typename U, unsigned int i>
    class TypeAt<TypeList<T, U>, i>
    {
        typedef typename TypeAt<U, i - 1>::Result Result;
    };
// ========== Find ==========
    template<typename Type>
    class Find<NullType, Type>
    {
        enum { Position = -1 };
    };

    template<typename T, typename Type>
    class Find<TypeList<T, Type>, T>
    {
        enum { Position = 0 };
    };

    template<typename T, typename U, typename Type>
    class Find<TypeList<T, U>, Type>
    {
        enum { Previous = Find<U, Type>::Position };
    public:
        enum { Position = Previous == -1 ? -1 : Previous + 1 };
    };
// ========== Append ==========
    template<>
    class Append<NullType, NullType>
    {
    public:
        typedef NullType Result;
    };

    template<typename Type>
    class Append<NullType, Type>
    {
    public:
        typedef TYPELIST_1(Type) Result;
    };

    template<typename T, typename U>
    class Append<NullType, TypeList<T, U> >
    {
    public:
        typedef TypeList<T, U> Result;
    };

    template<typename T, typename U, typename Type>
    class Append<TypeList<T, U>, Type>
    {
    public:
        typedef TypeList<T, typename Append<U, Type>::Result> Result;
    };
// ========== RemoveFirst ==========
    template<typename Type>
    class RemoveFirst<NullType, Type>
    {
    public:
        typedef NullType Result;
    };

    template<typename T, typename U>
    class RemoveFirst<TypeList<T, U>, T>
    {
    public:
        typedef U Result;
    };
    
    template<typename T, typename U, typename Type>
    class RemoveFirst<TypeList<T, U>, Type>
    {
    public:
        typedef TypeList<T, typename RemoveFirst<U, Type>::Result> Result;
    };
// ========== Unduplicate ==========
    template<>
    class Unduplicate<NullType>
    {
    public:
        typedef NullType Result;
    };

    template<typename T, typename U>
    class Unduplicate<TypeList<T, U> >
    {
        typedef typename Unduplicate<U>::Result Result1;
        typedef typename RemoveFirst<Result1, T>::Result Result2;
    public:
        typedef TypeList<T, Result2> Result;
    };
// ========== MostDerived ==========
    template<typename Type>
    class MostDerived<NullType, Type>
    {
    public:
        typedef Type Result;
    };

    template<typename T, typename U, typename Type>
    class MostDerived<TypeList<T, U>, Type>
    {
        typedef typename MostDerived<U, Type>::Result Candidate;
    public:
        typedef typename Utility::Select<Candidate, T, SUPERCLASS(T, Candidate)>::Result Result;
    };
// ========== Sub2Top ==========
    template<>
    class Sub2Top<NullType>
    {
    public:
        typedef NullType Result;
    };
};

