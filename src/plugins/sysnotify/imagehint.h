/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QByteArray>
#include <QMetaType>

class QImage;
class QDBusArgument;

namespace LC
{
namespace Sysnotify
{
	class ImageHint
	{
		int Width_;
		int Height_;
		int RowStride_;
		bool HasAlpha_;
		int Channels_;
		int BPS_;
		QByteArray Data_;

		friend QDBusArgument& operator<< (QDBusArgument&, const ImageHint&);
		friend const QDBusArgument& operator>> (const QDBusArgument&, ImageHint&);
	public:
		ImageHint () = default;
		ImageHint (const QImage&);
	};

	QDBusArgument& operator<< (QDBusArgument&, const ImageHint&);
	const QDBusArgument& operator>> (const QDBusArgument&, ImageHint&);
}
}

Q_DECLARE_METATYPE (LC::Sysnotify::ImageHint)
