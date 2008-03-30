#ifndef TYPELIST_H
#define TYPELIST_H

namespace Utility
{
    // If T is convertable to U
    template<typename T, typename U>
    class Convertable
    {
        typedef char SmallerTest;
        struct BiggerTest { SmallerTest InternalTest_ [2]; };
        SmallerTest Test (U);
        BiggerTest Test (...);
        static T Make ();
    public:
        enum { SameType = 0, Exists = (sizeof (Test (Make ())) == sizeof (SmallerTest)) };
    };

    template<typename T>
    class Convertable<T, T>
    {
    public:
        enum { SameType = 1, Exists = 1 };
    };

    template<typename T, typename U, bool flag>
    class Select
    {
    public:
        typedef T Result;
    };
    template<typename T, typename U>
    class Select<T, U, false>
    {
    public:
        typedef U Result;
    };
};

// If T is superclass of U
#define SUPERCLASS(T,U) (Utility::Convertable<const U*, const T*>::Exists && !Utility::Convertable<const T*, void*>::Exists)

class NullType {};
class EmptyType {};

template<typename T, typename U>
class TypeList
{
public:
    typedef T Head;
    typedef U Tail;
};

#define TYPELIST_1(t1)                                    TypeList<t1, NullType>
#define TYPELIST_2(t1, t2)                                TypeList<t1, TYPELIST_1(t2)>
#define TYPELIST_3(t1, t2, t3)                            TypeList<t1, TYPELIST_2(t2, t3)>
#define TYPELIST_4(t1, t2, t3, t4)                        TypeList<t1, TYPELIST_3(t2, t3, t4)>
#define TYPELIST_5(t1, t2, t3, t4, t5)                    TypeList<T1, TYPELIST_4(t2, t3, t4, t5)>
#define TYPELIST_6(t1, t2, t3, t4, t5, t6)                TypeList<t1, TYPELIST_5(t2, t3, t4, t5, t6)>
#define TYPELIST_7(t1, t2, t3, t4, t5, t6, t7)            TypeList<t1, TYPELIST_6(t2, t3, t4, t5, t6, t7)>
#define TYPELIST_8(t1, t2, t3, t4, t5, t6, t7, t8)        TypeList<t1, TYPELIST_7(t2, t3, t4, t5, t6, t7, t8)>
#define TYPELIST_9(t1, t2, t3, t4, t5, t6, t7, t8, t9)    TypeList<t1, TYPELIST_8(t2, t3, t4, t5, t6, t7, t8, t9)>
#define TYPELIST_10(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10)>
#define TYPELIST_11(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11)>
#define TYPELIST_12(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12)>
#define TYPELIST_13(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13)>
#define TYPELIST_14(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14)>
#define TYPELIST_15(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15)>
#define TYPELIST_16(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16)>
#define TYPELIST_17(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17)>
#define TYPELIST_18(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18)>
#define TYPELIST_19(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18, t19) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18, t19)>
#define TYPELIST_20(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18, t19, t20) \
    TypeList<t1, TYPELIST_9(t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18, t19, t20)>

namespace TypeLists
{
    template<typename List> class Length;
    template<typename List, unsigned int index> class TypeAt;
    template<typename List, typename Type> class Find;
    template<typename List, typename Type> class Append;
    template<typename List, typename Type> class RemoveFirst;
    template<typename List> class Unduplicate;
    template<typename List, typename Type> class MostDerived;
    template<typename List> class Sub2Top;
};

#endif

