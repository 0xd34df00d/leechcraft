/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC::Util::Views
{
	namespace detail
	{
		template<template<typename, typename> class PairType, typename I1, typename I2>
		struct ZipIterator
		{
			I1 It1_;
			const I1 It1End_;
			I2 It2_;
			const I2 It2End_;

			bool operator== (const ZipIterator& other) const
			{
				if (IsSentinel () || other.IsSentinel ())
					return IsSentinel () == other.IsSentinel ();

				return It1_ == other.It1_ && It2_ == other.It2_;
			}

			bool IsSentinel () const
			{
				return It1_ == It1End_ || It2_ == It2End_;
			}

			auto& operator++ ()
			{
				++It1_;
				++It2_;
				return *this;
			}

			auto& operator++ (int)
			{
				auto it = *this;

				++It1_;
				++It2_;

				return it;
			}

			auto operator* () const
			{
				return PairType { *It1_, *It2_ };
			}
		};
	}

	template<template<typename, typename> class PairType = QPair, typename C1, typename C2>
	auto Zip (C1&& c1, C2&& c2)
	{
		using ZIt = detail::ZipIterator<PairType, typename std::decay_t<C1>::const_iterator, typename std::decay_t<C2>::const_iterator>;
		struct Range
		{
			C1 C1_;
			C2 C2_;

			auto begin () const { return ZIt { C1_.cbegin (), C1_.cend (), C2_.cbegin (), C2_.cend () }; }
			auto end () const { return ZIt { C1_.cend (), C1_.cend (), C2_.cend (), C2_.cend () }; }
		};

		return Range { std::forward<C1> (c1), std::forward<C2> (c2) };
	}
}
