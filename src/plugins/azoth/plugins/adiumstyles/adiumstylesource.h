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
#include <QCache>
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
class ICLEntry;
class IAccount;
class IProxyObject;

namespace AdiumStyles
{
	class PackProxyModel;

	class AdiumStyleSource : public QObject
						   , public IChatStyleResourceSource
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IChatStyleResourceSource)

		std::shared_ptr<Util::ResourceLoader> StylesLoader_;
		IProxyObject *Proxy_;

		PackProxyModel *PackProxyModel_;

		mutable QHash<QWebFrame*, QString> Frame2Pack_;
		mutable QHash<QString, QList<QColor>> Coloring2Colors_;
		mutable QString LastPack_;

		QHash<QObject*, QWebFrame*> Msg2Frame_;

		mutable QHash<QWebFrame*, QObject*> Frame2LastContact_;
	public:
		AdiumStyleSource (IProxyObject*, QObject* = 0);

		QAbstractItemModel* GetOptionsModel () const;
		QUrl GetBaseURL (const QString&) const;
		QString GetHTMLTemplate (const QString&,
				const QString&, QObject*, QWebFrame*) const;
		bool AppendMessage (QWebFrame*, QObject*, const ChatMsgAppendInfo&);
		void FrameFocused (QWebFrame*);
		QStringList GetVariantsForPack (const QString&);
	private:
		void PercentTemplate (QString&, const QMap<QString, QString>&) const;
		void SubstituteUserIcon (QString&,
				const QString&, bool, ICLEntry*, IAccount*);
		QString ParseMsgTemplate (QString templ, const QString& path,
				QWebFrame*, QObject*, const ChatMsgAppendInfo&);
		QString GetMessageID (QObject*);
	private slots:
		void handleMessageDelivered ();
		void handleMessageDestroyed ();
		void handleFrameDestroyed ();
	};
}
}
}
