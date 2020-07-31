/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "baseemoticonssource.h"
#include <QtDebug>
#include <util/sys/resourceloader.h>
#include <util/sll/containerconversions.h>
#include <util/sll/qtutil.h>

namespace LC
{
namespace Azoth
{
namespace NativeEmoticons
{
	BaseEmoticonsSource::BaseEmoticonsSource (const QString& suffix, QObject *parent)
	: QObject { parent }
	, EmoLoader_ { new Util::ResourceLoader { "azoth/emoticons/" + suffix, this } }
	{
		EmoLoader_->AddGlobalPrefix ();
		EmoLoader_->AddLocalPrefix ();
	}

	QAbstractItemModel* BaseEmoticonsSource::GetOptionsModel () const
	{
		return EmoLoader_->GetSubElemModel ();
	}

	QSet<QString> BaseEmoticonsSource::GetEmoticonStrings (const QString& pack) const
	{
		return Util::AsSet (ParseFile (pack).keys ());
	}

	QHash<QImage, QString> BaseEmoticonsSource::GetReprImages (const QString& pack) const
	{
		QHash<QImage, QString> result;

		QSet<QString> knownPaths;
		for (const auto& pair : Util::Stlize (ParseFile (pack)))
		{
			const auto& path = pair.second;
			if (knownPaths.contains (path))
				continue;

			knownPaths << path;

			const auto& fullPath = EmoLoader_->GetIconPath (pack + "/" + path);
			const QImage img { fullPath };
			if (img.isNull ())
			{
				qWarning () << Q_FUNC_INFO
						<< path
						<< "in pack"
						<< pack
						<< "is null, got path:"
						<< fullPath;
				continue;
			}

			result [img] = pair.first;
		}

		return result;
	}

	QByteArray BaseEmoticonsSource::GetImage (const QString& pack, const QString& smile) const
	{
		const auto& hash = ParseFile (pack);
		if (!hash.contains (smile))
			return {};

		const auto& path = EmoLoader_->GetIconPath (pack + "/" + hash [smile]);
		QFile file { path };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< "for"
					<< pack
					<< smile
					<< file.errorString ();
			return {};
		}

		return file.readAll ();
	}
}
}
}
