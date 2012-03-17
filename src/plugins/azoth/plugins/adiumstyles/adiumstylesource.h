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

#pragma once

#include <memory>
#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QColor>
#include <interfaces/ichatstyleresourcesource.h>
#include "plistparser.h"

namespace LeechCraft
{
namespace Util
{
	class ResourceLoader;
}

namespace Azoth
{
class IMessage;
class ICLEntry;
class IProxyObject;

namespace AdiumStyles
{
	class PackProxyModel;

	class AdiumStyleSource : public QObject
						   , public IChatStyleResourceSource
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IChatStyleResourceSource)

		std::shared_ptr<Util::ResourceLoader> StylesLoader_;
		IProxyObject *Proxy_;

		PackProxyModel *PackProxyModel_;

		mutable QHash<QWebFrame*, QString> Frame2Pack_;
		mutable QHash<QString, QList<QColor>> Coloring2Colors_;
		mutable QHash<QString, PListParser_ptr> PListParsers_;
		mutable QString LastPack_;

		QHash<QObject*, QWebFrame*> Msg2Frame_;

		mutable QHash<QWebFrame*, QObject*> Frame2LastContact_;
	public:
		AdiumStyleSource (IProxyObject*, QObject* = 0);

		QAbstractItemModel* GetOptionsModel () const;
		QUrl GetBaseURL (const QString&) const;
		QString GetHTMLTemplate (const QString&, QObject*, QWebFrame*) const;
		bool AppendMessage (QWebFrame*, QObject*, const ChatMsgAppendInfo&);
		void FrameFocused (QWebFrame*);
	private:
		void PercentTemplate (QString&, const QMap<QString, QString>&) const;
		void ParseGlobalTemplate (QString& templ, ICLEntry*) const;
		QString ParseMsgTemplate (QString templ, const QString& path,
				QWebFrame*, QObject*, const ChatMsgAppendInfo&);
		QList<QColor> CreateColors (const QString&);
		QString GetMessageID (QObject*);
		QImage GetDefaultAvatar () const;
		PListParser_ptr GetPListParser (const QString&) const;
	private slots:
		void handleMessageDelivered ();
		void handleMessageDestroyed ();
		void handleFrameDestroyed ();
	};
}
}
}
