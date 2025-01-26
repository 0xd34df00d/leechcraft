/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imagehint.h"
#include <QImage>
#include <QDBusArgument>

namespace LC
{
namespace Sysnotify
{
	ImageHint::ImageHint (const QImage& image)
	: Width_ { image.width () }
	, Height_ { image.height () }
	, RowStride_ { static_cast<int> (image.bytesPerLine ()) }
	, HasAlpha_ { image.hasAlphaChannel () }
	, Channels_ { HasAlpha_ ? 4 : 3 }
	, BPS_ { image.depth () / Channels_ }
	, Data_ { reinterpret_cast<char*> (image.rgbSwapped ().bits ()), static_cast<int> (image.sizeInBytes ()) }
	{
	}

	QDBusArgument& operator<< (QDBusArgument& arg, const ImageHint& hint)
	{
		arg.beginStructure ();
		arg << hint.Width_
				<< hint.Height_
				<< hint.RowStride_
				<< hint.HasAlpha_
				<< hint.BPS_
				<< hint.Channels_
				<< hint.Data_;
		arg.endStructure ();
		return arg;
	}

	const QDBusArgument& operator>> (const QDBusArgument& arg, ImageHint& hint)
	{
		arg.beginStructure ();
		arg >> hint.Width_
				>> hint.Height_
				>> hint.RowStride_
				>> hint.HasAlpha_
				>> hint.BPS_
				>> hint.Channels_
				>> hint.Data_;
		arg.endStructure ();
		return arg;
	}
}
}
