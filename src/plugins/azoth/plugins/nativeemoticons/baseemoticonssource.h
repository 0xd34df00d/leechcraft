/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_NATIVEEMOTICONS_BASEEMOTICONSSOURCE_H
#define PLUGINS_AZOTH_PLUGINS_NATIVEEMOTICONS_BASEEMOTICONSSOURCE_H
#include <memory>
#include <QObject>
#include <QSet>
#include <interfaces/iresourceplugin.h>

namespace LeechCraft
{
namespace Util
{
	class ResourceLoader;
}
namespace Azoth
{
namespace NativeEmoticons
{
	class BaseEmoticonsSource : public QObject
							  , public IEmoticonResourceSource
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IEmoticonResourceSource);
	protected:
		std::shared_ptr<Util::ResourceLoader> EmoLoader_;

		typedef QHash<QString, QString> String2Filename_t;
	public:
		BaseEmoticonsSource (const QString&, QObject* = 0);

		QAbstractItemModel* GetOptionsModel () const;
		QSet<QString> GetEmoticonStrings (const QString&) const;
		QHash<QImage, QString> GetReprImages (const QString&) const;
		QByteArray GetImage (const QString&, const QString&) const;
	protected:
		virtual String2Filename_t ParseFile (const QString&) const = 0;
	};
}
}
}

#endif
