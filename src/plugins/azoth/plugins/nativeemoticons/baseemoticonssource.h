/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_NATIVEEMOTICONS_BASEEMOTICONSSOURCE_H
#define PLUGINS_AZOTH_PLUGINS_NATIVEEMOTICONS_BASEEMOTICONSSOURCE_H
#include <memory>
#include <QObject>
#include <QSet>
#include <interfaces/azoth/iresourceplugin.h>

namespace LC
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
		Q_INTERFACES (LC::Azoth::IEmoticonResourceSource)
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
