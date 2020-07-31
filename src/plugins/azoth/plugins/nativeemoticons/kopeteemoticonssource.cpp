/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kopeteemoticonssource.h"
#include <QtDebug>
#include <QDomComment>
#include <util/sys/resourceloader.h>

namespace LC
{
namespace Azoth
{
namespace NativeEmoticons
{
	KopeteEmoticonsSource::KopeteEmoticonsSource (QObject *parent)
	: BaseEmoticonsSource ("custom/kopete/", parent)
	{
	}

	KopeteEmoticonsSource::String2Filename_t
			KopeteEmoticonsSource::ParseFile (const QString& pack) const
	{
		if (CachedPack_ == pack && !IconCache_.isEmpty ())
			return IconCache_;

		Util::QIODevice_ptr dev = EmoLoader_->Load (pack + "/emoticons.xml");
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
					<< pack + "/" + "emoticons.xml:"
					<< dev->errorString ();
			return QHash<QString, QString> ();
		}

		QDomDocument smileXml ("smileXml");

		if (!smileXml.setContent (dev.get ()))
		{
			qWarning () << "Unable to read xml from file";
			return QHash<QString, QString> ();
		}

		QDomElement docElem = smileXml.documentElement ();
		QDomNode n = docElem.firstChild ();
		while (!n.isNull ())
		{
			QDomElement e = n.toElement ();
			n = n.nextSibling ();
			if (e.isNull ())
				continue;

			QDomNode chn = e.firstChild ();
			while (!chn.isNull ())
			{
				QDomElement smileElem = chn.toElement ();
				IconCache_ [smileElem.text ()] = e.attribute ("file");
				chn = chn.nextSibling ();
			}
		}

		CachedPack_ = pack;
		return IconCache_;
	}
}
}
}
