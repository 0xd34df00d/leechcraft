/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QHash>
#include <QStringList>
#include <QXmppDiscoveryIq.h>
#include <interfaces/azoth/ihaveservicediscovery.h>

class QStandardItemModel;
class QStandardItem;

namespace LC
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
		Q_INTERFACES (LC::Azoth::ISDSession)

		QString Query_;

		SDModel * const Model_;
		GlooxAccount * const Account_;
		QHash<QString, QHash<QString, QStandardItem*>> JID2Node2Item_;

		struct ItemInfo
		{
			QStringList Caps_;
			QList<QXmppDiscoveryIq::Identity> Identities_;
			QString JID_;
			QString Node_;
		};
		QHash<QStandardItem*, ItemInfo> Item2Info_;

		using ItemAction_t = std::function<void (ItemInfo)>;
		QHash<QByteArray, ItemAction_t> ID2Action_;

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
		QString GetQuery () const;
		QAbstractItemModel* GetRepresentationModel () const;
		QList<QPair<QByteArray, QString>> GetActionsFor (const QModelIndex&);
		void ExecuteAction (const QModelIndex&, const QByteArray&);

		void HandleInfo (const QXmppDiscoveryIq&);
		void HandleItems (const QXmppDiscoveryIq&);

		void QueryItem (QStandardItem*);
	private:
		void ViewVCard (const ItemInfo&);
		void AddToRoster (const ItemInfo&);
		void Register (const ItemInfo&);
		void ExecuteAdHoc (const ItemInfo&);
		void JoinConference (const ItemInfo&);
	private slots:
		void handleRegistrationForm (const QXmppIq&);
	};
}
}
}
