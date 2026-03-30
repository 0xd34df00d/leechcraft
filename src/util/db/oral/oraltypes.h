/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <type_traits>
#include <util/sll/typelist.h>
#include <util/sll/typegetter.h>

namespace LC
{
namespace Util
{
namespace oral
{
	struct NoAutogen;

	template<typename T>
	concept Indirect = T::IsIndirect;

	namespace detail
	{
		template<typename T>
		struct WrapDirect { using value_type = T; };

		template<typename T>
		using UnwrapIndirect_t = std::conditional_t<Indirect<T>,
				T,
				WrapDirect<T>>::value_type;
	}

	template<typename T, typename Concrete>
	struct IndirectHolderBase
	{
		constexpr static bool IsIndirect = true;

		using base_type = T;
		using value_type = detail::UnwrapIndirect_t<T>;

		T Val_;

		IndirectHolderBase () = default;

		IndirectHolderBase (value_type val)
		: Val_ { std::move (val) }
		{
		}

		template<typename Arg, typename... Args>
			requires std::constructible_from<value_type, Arg&&, Args&&...> &&
					(sizeof... (Args) > 0 || !std::derived_from<std::decay_t<Arg>, IndirectHolderBase>)
		IndirectHolderBase (Arg&& arg, Args&&... args)
		: Val_ { std::forward<Arg> (arg), std::forward<Args> (args)... }
		{
		}

		Concrete& operator= (value_type val)
		{
			Val_ = std::move (val);
			return static_cast<Concrete&> (*this);
		}

		explicit (false) operator value_type () const
		{
			return Val_;
		}

		const value_type& operator* () const
		{
			if constexpr (Indirect<T>)
				return *Val_;
			else
				return Val_;
		}

		auto operator-> () const
		{
			return &Val_;
		}

		friend constexpr bool operator== (const Concrete& c1, const Concrete& c2)
		{
			return c1.Val_ == c2.Val_;
		}

		friend constexpr bool operator== (const Concrete& c, const value_type& v)
		{
			return c.Val_ == v;
		}

		friend constexpr auto operator<=> (const Concrete& c1, const Concrete& c2)
			requires std::three_way_comparable<T>
		{
			return c1.Val_ <=> c2.Val_;
		}

		friend constexpr auto operator<=> (const Concrete& c, const value_type& v)
			requires std::three_way_comparable<T>
		{
			return c.Val_ <=> v;
		}
	};

	template<typename T, typename... Tags>
	struct PKey : IndirectHolderBase<T, PKey<T, Tags...>>
	{
		using PKey::IndirectHolderBase::IndirectHolderBase;
	};

	template<typename T, typename... Args>
	using PKeyValue_t = PKey<T, Args...>::value_type;

	template<typename T>
	struct Unique : IndirectHolderBase<T, Unique<T>>
	{
		using Unique::IndirectHolderBase::IndirectHolderBase;
	};

	template<typename T>
	using UniqueValue_t = Unique<T>::value_type;

	template<typename T>
	struct NotNull : IndirectHolderBase<T, NotNull<T>>
	{
		using NotNull::IndirectHolderBase::IndirectHolderBase;
	};

	template<typename T>
	using NotNullValue_t = NotNull<T>::value_type;

	template<typename T>
	using UniqueNotNull = Unique<NotNull<T>>;

	namespace detail
	{
		template<typename T>
		struct IsReferencesTarget : std::false_type {};

		template<typename U, typename... Tags>
		struct IsReferencesTarget<PKey<U, Tags...>> : std::true_type {};

		template<typename U>
		struct IsReferencesTarget<Unique<U>> : std::true_type {};
	}

	template<auto Ptr>
	struct References : IndirectHolderBase<typename MemberPtrType_t<Ptr>::value_type, References<Ptr>>
	{
		constexpr static bool IsReferences = true;

		using member_type = MemberPtrType_t<Ptr>;
		static_assert (detail::IsReferencesTarget<member_type>::value, "References<> element must refer to a PKey<> element");

		using References::IndirectHolderBase::IndirectHolderBase;

		template<typename T, typename... Tags>
		References (const PKey<T, Tags...>& key)
		: References::IndirectHolderBase (key)
		{
		}

		template<typename T, typename... Tags>
		References& operator= (const PKey<T, Tags...>& key)
		{
			this->Val_ = key;
			return *this;
		}
	};

	template<auto Ptr>
	using ReferencesValue_t = References<Ptr>::value_type;

	template<auto... Fields>
	struct PrimaryKey {};

	template<auto... Fields>
	struct UniqueSubset {};

	template<typename... Args>
	using Constraints = Typelist<Args...>;

	template<auto... Fields>
	struct Index {};

	template<typename... Args>
	using Indices = Typelist<Args...>;

	template<typename T>
	concept ForeignKey = T::IsReferences;

	struct InsertAction
	{
		constexpr static struct DefaultTag {} Default {};
		constexpr static struct IgnoreTag {} Ignore {};

		struct Replace
		{
			template<auto... Ptrs>
			struct FieldsType {};

			template<auto... Ptrs>
			constexpr static FieldsType<Ptrs...> Fields {};

			constexpr static struct WholeType {} Whole {};
		};
	};
}
}
}
