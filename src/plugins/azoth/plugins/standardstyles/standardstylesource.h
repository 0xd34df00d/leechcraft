/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QColor>
#include <interfaces/azoth/ichatstyleresourcesource.h>

namespace LC
{
namespace Util
{
	class ResourceLoader;
}

namespace Azoth
{
class IMessage;
class IProxyObject;

namespace StandardStyles
{
	class StandardStyleSource : public QObject
							  , public IChatStyleResourceSource
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IChatStyleResourceSource)

		std::shared_ptr<Util::ResourceLoader> StylesLoader_;

		QMap<QWebFrame*, bool> IsLastMsgRead_;
		IProxyObject *Proxy_;

		mutable QHash<QString, QList<QColor>> Coloring2Colors_;
		mutable QString LastPack_;

		QHash<QObject*, QWebFrame*> Msg2Frame_;
	public:
		StandardStyleSource (IProxyObject*, QObject* = 0);

		QAbstractItemModel* GetOptionsModel () const;
		QUrl GetBaseURL (const QString&) const;
		QString GetHTMLTemplate (const QString&,
				const QString&, QObject*, QWebFrame*) const;
		bool AppendMessage (QWebFrame*, QObject*, const ChatMsgAppendInfo&);
		void FrameFocused (QWebFrame*);
		QStringList GetVariantsForPack (const QString&);
	private:
		QList<QColor> CreateColors (const QString&, QWebFrame*);
		QString GetMessageID (QObject*);
		QString GetStatusImage (const QString&);
		void HandleMessageDestroyed (QObject*);
		void HandleFrameDestroyed (QObject*);
	private slots:
		void handleMessageDelivered ();
	};
}
}
}
