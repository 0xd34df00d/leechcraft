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

	template<typename T, typename Concrete>
	struct IndirectHolderBase
	{
		using value_type = T;

		T Val_;

		IndirectHolderBase () = default;

		IndirectHolderBase (T val)
		: Val_ { val }
		{
		}

		template<typename U = T, typename Sub = typename U::value_type>
		IndirectHolderBase (Sub val)
		: Val_ { val }
		{
		}

		Concrete& operator= (T val)
		{
			Val_ = val;
			return static_cast<Concrete&> (*this);
		}

		operator value_type () const
		{
			return Val_;
		}

		const value_type& operator* () const
		{
			return Val_;
		}

		const value_type* operator-> () const
		{
			return &Val_;
		}
	};

	template<typename T, typename... Tags>
	struct PKey : IndirectHolderBase<T, PKey<T, Tags...>>
	{
		using PKey::IndirectHolderBase::IndirectHolderBase;
	};

	template<typename T, typename... Args>
	using PKeyValue_t = typename PKey<T, Args...>::value_type;

	template<typename T>
	struct Unique : IndirectHolderBase<T, Unique<T>>
	{
		using Unique::IndirectHolderBase::IndirectHolderBase;
	};

	template<typename T>
	using UniqueValue_t = typename Unique<T>::value_type;

	template<typename T>
	struct NotNull : IndirectHolderBase<T, NotNull<T>>
	{
		using NotNull::IndirectHolderBase::IndirectHolderBase;
	};

	template<typename T>
	using NotNullValue_t = typename NotNull<T>::value_type;

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
	using ReferencesValue_t = typename References<Ptr>::value_type;

	template<size_t... Fields>
	struct PrimaryKey {};

	template<size_t... Fields>
	struct UniqueSubset {};

	template<typename... Args>
	using Constraints = Typelist<Args...>;

	template<auto... Fields>
	struct Index;

	template<typename... Args>
	using Indices = Typelist<Args...>;

	template<typename T>
	struct IsIndirect : std::false_type {};

	template<typename T, typename... Args>
	struct IsIndirect<PKey<T, Args...>> : std::true_type {};

	template<typename T>
	struct IsIndirect<Unique<T>> : std::true_type {};

	template<typename T>
	struct IsIndirect<NotNull<T>> : std::true_type {};

	template<auto Ptr>
	struct IsIndirect<References<Ptr>> : std::true_type {};

	struct InsertAction
	{
		constexpr inline static struct DefaultTag {} Default {};
		constexpr inline static struct IgnoreTag {} Ignore {};

		struct Replace
		{
			template<auto... Ptrs>
			struct FieldsType {};

			template<auto... Ptrs>
			constexpr inline static FieldsType<Ptrs...> Fields {};

			constexpr inline static struct PKeyType {} PKey {};
		};
	};
}
}
}
