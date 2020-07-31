/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nativeemoticonssource.h"
#include <QtDebug>
#include <util/sys/resourceloader.h>

namespace LC
{
namespace Azoth
{
namespace NativeEmoticons
{
	NativeEmoticonsSource::NativeEmoticonsSource (QObject *parent)
	: BaseEmoticonsSource ("native/", parent)
	{
	}

	NativeEmoticonsSource::String2Filename_t NativeEmoticonsSource::ParseFile (const QString& pack) const
	{
		if (CachedPack_ == pack && !IconCache_.isEmpty ())
			return IconCache_;

		Util::QIODevice_ptr dev = EmoLoader_->Load (pack + "/" + "mapping.txt");
		if (!dev)
		{
			qWarning () << Q_FUNC_INFO
					<< "null QIODevice for"
					<< pack;
			return QHash<QString, QString> ();
		}

		if (!dev->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open QIODevice for"
					<< pack + "/" + "mapping.txt:"
					<< dev->errorString ();
			return QHash<QString, QString> ();
		}

		while (!dev->atEnd ())
		{
			const QByteArray& arr = dev->readLine (16384).trimmed ();
			const int idx = arr.indexOf (' ');
			if (idx == -1)
			{
				qWarning () << Q_FUNC_INFO
						<< "incorrect line for"
						<< pack
						<< arr;
				continue;
			}

			QByteArray name = arr.left (idx);
			if (name.endsWith (".png") || name.endsWith (".gif"))
				name.chop (4);
			IconCache_ [arr.mid (idx + 1)] = name;
		}
		CachedPack_ = pack;
		return IconCache_;
	}
}
}
}
