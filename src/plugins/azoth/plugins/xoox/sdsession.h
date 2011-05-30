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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_SDSESSION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_SDSESSION_H
#include <QObject>
#include <QHash>
#include <interfaces/ihaveservicediscovery.h>

class QStandardItemModel;
class QStandardItem;
class QXmppDiscoveryIq;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;
	class SDModel;

	class SDSession : public QObject
					, public ISDSession
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ISDSession)
		
		SDModel *Model_;
		GlooxAccount *Account_;
		QHash<QString, QHash<QString, QStandardItem*> > JID2Node2Item_;
		
		enum Columns
		{
			CName,
			CJID,
			CNode
		};
	public:
		enum DataRoles
		{
			DRFetchedMore = Qt::UserRole + 1,
			DRJID,
			DRNode
		};
		SDSession (GlooxAccount*);
		
		void SetQuery (const QString&);
		QAbstractItemModel* GetRepresentationModel () const;
		QList<QPair<QByteArray, QString> > GetActionsFor (const QModelIndex&);
		void ExecuteAction (const QModelIndex&, const QByteArray&);
		
		void HandleInfo (const QXmppDiscoveryIq&);
		void HandleItems (const QXmppDiscoveryIq&);
		
		void QueryItem (QStandardItem*);
	};
}
}
}

#endif
