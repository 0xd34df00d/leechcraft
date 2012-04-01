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

#ifndef PLUGINS_AZOTH_PLUGINS_STANDARDSTYLES_STANDARDSTYLESOURCE_H
#define PLUGINS_AZOTH_PLUGINS_STANDARDSTYLES_STANDARDSTYLESOURCE_H
#include <memory>
#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QColor>
#include <interfaces/ichatstyleresourcesource.h>

namespace LeechCraft
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
		Q_INTERFACES (LeechCraft::Azoth::IChatStyleResourceSource)

		std::shared_ptr<Util::ResourceLoader> StylesLoader_;

		QMap<QWebFrame*, bool> HasBeenAppended_;
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
		QList<QColor> CreateColors (const QString&);
		QString GetMessageID (QObject*);
		QString GetStatusImage (const QString&);
	private slots:
		void handleMessageDelivered ();
		void handleMessageDestroyed ();
		void handleFrameDestroyed ();
	};
}
}
}

#endif
