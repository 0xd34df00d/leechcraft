/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>

class QByteArray;

namespace LC
{
namespace Monocle
{
namespace Dik
{
	class Decompressor;
	typedef std::shared_ptr<Decompressor> Decompressor_ptr;

	class MobiParser;

	class Decompressor
	{
	public:
		enum class Type
		{
			None,
			RLE,
			Huff
		};

		virtual ~Decompressor ();

		virtual QByteArray operator() (const QByteArray&) = 0;

		static Decompressor_ptr Create (Type, const MobiParser*);
	};
}
}
}
