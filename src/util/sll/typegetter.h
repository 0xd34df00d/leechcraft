/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <tuple>

namespace LC::Util
{
	namespace detail
	{
		template<typename R, typename... Args>
		std::tuple<R, Args...> TypeGetter (R (*) (Args...));

		template<typename F>
		auto TypeGetter (F&& f) -> decltype (TypeGetter (+f));

		template<typename C, typename R, typename... Args>
		std::tuple<R, Args...> TypeGetter (R (C::*) (Args...) const);

		template<typename C, typename R, typename... Args>
		std::tuple<R, Args...> TypeGetter (R (C::*) (Args...));

		template<typename C>
		decltype (TypeGetter (&C::operator ())) TypeGetter (const C& c);
	}

	template<typename F, size_t Idx>
	using ArgType_t = std::tuple_element_t<Idx + 1, decltype (detail::TypeGetter (*static_cast<F*> (nullptr)))>;

	template<typename F>
	using RetType_t = std::tuple_element_t<0, decltype (detail::TypeGetter (*static_cast<F*> (nullptr)))>;

	namespace detail
	{
		template<typename>
		struct DecomposeMemberPtr
		{
		};

		template<typename R, typename C>
		struct DecomposeMemberPtr<R (C::*)>
		{
			using Value_t = R;
			using StructType_t = C;
		};
	}

	template<typename PtrType>
	using MemberTypeType_t = typename detail::DecomposeMemberPtr<PtrType>::Value_t;

	template<typename PtrType>
	using MemberTypeStruct_t = typename detail::DecomposeMemberPtr<PtrType>::StructType_t;

	template<auto Ptr>
	using MemberPtrType_t = MemberTypeType_t<decltype (Ptr)>;

	template<auto Ptr>
	using MemberPtrStruct_t = MemberTypeStruct_t<decltype (Ptr)>;
}
