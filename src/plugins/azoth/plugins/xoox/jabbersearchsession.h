/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_JABBERSEARCHSESSION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_JABBERSEARCHSESSION_H
#include <QObject>
#include <interfaces/azoth/ihavesearch.h>
#include "xeps/jabbersearchmanager.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;

	class JabberSearchSession : public QObject
							  , public ISearchSession
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ISearchSession)

		QStandardItemModel *Model_;

		JabberSearchManager& SM_;

		QString CurrentServer_;
	public:
		JabberSearchSession (GlooxAccount *acc);

		void RestartSearch (QString);
		QAbstractItemModel* GetRepresentationModel () const;
	private slots:
		void handleGotItems (const QString&, const QList<JabberSearchManager::Item>&);
		void handleGotSearchFields (const QString&, const QXmppElement&);
		void handleGotError (const QXmppIq&);
	};
}
}
}

#endif
