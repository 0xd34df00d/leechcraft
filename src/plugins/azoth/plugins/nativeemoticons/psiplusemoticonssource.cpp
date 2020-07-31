/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "psiplusemoticonssource.h"
#include <QtDebug>
#include <QDomComment>
#include <util/sys/resourceloader.h>

namespace LC
{
namespace Azoth
{
namespace NativeEmoticons
{
	PsiPlusEmoticonsSource::PsiPlusEmoticonsSource (QObject *parent)
	: BaseEmoticonsSource ("custom/psiplus/", parent)
	{
	}

	PsiPlusEmoticonsSource::String2Filename_t PsiPlusEmoticonsSource::ParseFile (const QString& pack) const
	{
		if (CachedPack_ == pack && !IconCache_.isEmpty ())
			return IconCache_;

		Util::QIODevice_ptr dev = EmoLoader_->Load (pack + "/icondef.xml");
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
					<< pack + "/icondef.xml:"
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

			if (e.isNull () || e.tagName () == "meta")
				continue;

			QDomNode chn = e.firstChild ();
			QStringList smiles = QStringList ();
			while (!chn.isNull ())
			{
				QDomElement smileElem = chn.toElement ();
				if (smileElem.tagName () == "text")
					smiles << smileElem.text ();
				else if (smileElem.tagName () == "object")
				{
					auto name = smileElem.text ();
					if (name.endsWith (".png") || name.endsWith (".gif"))
						name.chop (4);
					for (const auto& str : smiles)
						IconCache_ [str] = name;
				}
				chn = chn.nextSibling ();
			}
		}

		CachedPack_ = pack;
		return IconCache_;
	}
}
}
}
