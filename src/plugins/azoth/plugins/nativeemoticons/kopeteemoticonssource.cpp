/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "kopeteemoticonssource.h"
#include <QtDebug>
#include <QDomComment>
#include <util/resourceloader.h>

namespace LeechCraft
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
