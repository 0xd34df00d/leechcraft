/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "nativeemoticonssource.h"
#include <QtDebug>
#include <plugininterface/resourceloader.h>

namespace LeechCraft
{
namespace Azoth
{
namespace NativeEmoticons
{
	NativeEmoticonsSource::NativeEmoticonsSource (QObject *parent)
	: QObject (parent)
	, EmoLoader_ (new Util::ResourceLoader ("azoth/emoticons/native/", this))
	{
		EmoLoader_->AddGlobalPrefix ();
		EmoLoader_->AddLocalPrefix ();
	}
	
	QAbstractItemModel* NativeEmoticonsSource::GetOptionsModel () const
	{
		return EmoLoader_->GetSubElemModel ();
	}
	
	QSet<QString> NativeEmoticonsSource::GetEmoticonStrings (const QString& pack) const
	{
		return ParseFile (pack).keys ().toSet ();
	}
	
	QHash<QImage, QString> NativeEmoticonsSource::GetReprImages (const QString& pack) const
	{
		QHash<QImage, QString> result;

		const String2Filename_t& hash = ParseFile (pack);
		const QSet<QString>& uniqueImgs = hash.values ().toSet ();
		Q_FOREACH (const QString& imgPath, uniqueImgs)
		{
			const QString& fullPath = EmoLoader_->GetIconPath (pack + "/" + imgPath);
			const QImage& img = QImage (fullPath);
			if (img.isNull ())
			{
				qWarning () << Q_FUNC_INFO
						<< imgPath
						<< "in pack"
						<< pack
						<< "is null, got path:"
						<< fullPath;
				continue;
			}
			
			result [img] = hash.key (imgPath);
		}
		
		return result;
	}
	
	QByteArray NativeEmoticonsSource::GetImage (const QString& pack, const QString& smile) const
	{
		const String2Filename_t& hash = ParseFile (pack);
		if (!hash.contains (smile))
			return QByteArray ();
		
		const QString& path = EmoLoader_->GetIconPath (pack + "/" + hash [smile]);
		QFile file (path);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< "for"
					<< pack
					<< smile
					<< file.errorString ();
			return QByteArray ();
		}
		
		return file.readAll ();
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
			if (name.endsWith (".png"))
				name.chop (4);
			IconCache_ [arr.mid (idx + 1)] = name;
		}
		CachedPack_ = pack;
		return IconCache_;
	}
}
}
}
