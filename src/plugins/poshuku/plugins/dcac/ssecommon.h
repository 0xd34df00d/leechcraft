/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/intseq.h>

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	template<int Alignment, typename F>
	void HandleLoopBegin (const uchar * const scanline, int width, int& x, int& bytesCount, F&& f)
	{
		const int beginUnaligned = std::bit_cast<uintptr_t> (scanline) % Alignment;
		bytesCount = width * 4;
		if (beginUnaligned)
		{
			x += Alignment - beginUnaligned;
			bytesCount -= Alignment - beginUnaligned;

			for (int i = 0; i < Alignment - beginUnaligned; i += 4)
				f (i);
		}

		bytesCount -= bytesCount % Alignment;
	}

	template<typename F>
	void HandleLoopEnd (int width, int x, F&& f)
	{
		for (int i = x; i < width * 4; i += 4)
			f (i);
	}

	namespace detail
	{
		template<char From, char To, char ByteNum, char BytesPerElem>
		struct GenSeq;

		template<char From, char To, char ByteNum, char BytesPerElem>
		using EpiSeq = typename GenSeq<From, To, ByteNum, BytesPerElem>::type;

		template<char From, char To, char ByteNum, char BytesPerElem>
		struct GenSeq
		{
			using type = Util::IntSeq::Concat<EpiSeq<From, From, ByteNum, BytesPerElem>, EpiSeq<From - 1, To, ByteNum, BytesPerElem>>;
		};

		template<char E, char ByteNum, char BytesPerElem>
		struct GenSeq<E, E, ByteNum, BytesPerElem>
		{
			using type = Util::IntSeq::Concat<
					Util::IntSeq::Repeat<uchar, 0x80, BytesPerElem - ByteNum - 1>,
					std::integer_sequence<uchar, E>,
					Util::IntSeq::Repeat<uchar, 0x80, ByteNum>
				>;
		};

		template<size_t BytesCount, size_t Bucket, char ByteNum, char BytesPerElem>
		struct GenRevSeqS
		{
			static constexpr uchar EndValue = BytesCount * BytesPerElem - BytesPerElem;
			static constexpr auto TotalCount = BytesCount * BytesPerElem;
			static constexpr auto BeforeEmpty = BytesCount * Bucket;
			static constexpr auto AfterEmpty = TotalCount - BytesCount - BeforeEmpty;

			static_assert (AfterEmpty >= 0, "negative sequel size");
			static_assert (BeforeEmpty >= 0, "negative prequel size");

			template<uchar... Is>
			static auto BytesImpl (std::integer_sequence<uchar, Is...>)
			{
				return std::integer_sequence<uchar, (EndValue - Is * BytesPerElem + ByteNum)...> {};
			}

			using type = Util::IntSeq::Concat<
					Util::IntSeq::Repeat<uchar, 0x80, AfterEmpty>,
					decltype (BytesImpl (std::make_integer_sequence<uchar, BytesCount> {})),
					Util::IntSeq::Repeat<uchar, 0x80, BeforeEmpty>
				>;
		};

		template<size_t BytesCount, size_t Bucket, char ByteNum, char BytesPerElem>
		using GenRevSeq = typename GenRevSeqS<BytesCount, Bucket, ByteNum, BytesPerElem>::type;

		template<uint16_t>
		struct MaskTag {};

		using Mask128 = MaskTag<128>;
		using Mask256 = MaskTag<256>;
	}

	constexpr detail::Mask128 Mask128;
	constexpr detail::Mask256 Mask256;

	namespace detail
	{
		template<uchar... Is>
		__attribute__ ((target ("sse2")))
		auto MakeMaskImpl (Mask128, std::integer_sequence<uchar, Is...>)
		{
			return _mm_set_epi8 (Is...);
		}

		template<uchar... Is>
		__attribute__ ((target ("avx")))
		auto MakeMaskImpl (Mask256, std::integer_sequence<uchar, Is...>)
		{
			return _mm256_set_epi8 (Is..., Is...);
		}

		template<char From, char To, char ByteNum>
		auto MakeMaskSeq ()
		{
			static_assert (!(16 % (From - To + 1)), "integral byte count expected");

			constexpr char BytesPerElem = 16 / (From - To + 1);
			return EpiSeq<From, To, ByteNum, BytesPerElem> {};
		}

		template<size_t BytesCount, size_t Bucket, char ByteNum = 0>
		auto MakeRevMaskSeq ()
		{
			static_assert (!(16 % BytesCount), "integral byte count expected");

			constexpr char BytesPerElem = 16 / BytesCount;
			return GenRevSeq<BytesCount, Bucket, ByteNum, BytesPerElem> {};
		}
	}

	template<char From, char To, char ByteNum = 0>
	auto MakeMask (detail::Mask128 tag)
	{
		return MakeMaskImpl (tag, detail::MakeMaskSeq<From, To, ByteNum> ());
	}

	template<char From, char To, char ByteNum = 0>
	__attribute__ ((target ("avx")))
	auto MakeMask (detail::Mask256 tag)
	{
		return MakeMaskImpl (tag, detail::MakeMaskSeq<From, To, ByteNum> ());
	}

	template<size_t BytesCount, size_t Bucket, char ByteNum = 0>
	auto MakeRevMask (detail::Mask128 tag)
	{
		return MakeMaskImpl (tag, detail::MakeRevMaskSeq<BytesCount, Bucket, ByteNum> ());
	}

	template<size_t BytesCount, size_t Bucket, char ByteNum = 0>
	__attribute__ ((target ("avx")))
	auto MakeRevMask (detail::Mask256 tag)
	{
		return MakeMaskImpl (tag, detail::MakeRevMaskSeq<BytesCount, Bucket, ByteNum> ());
	}
}
}
}
