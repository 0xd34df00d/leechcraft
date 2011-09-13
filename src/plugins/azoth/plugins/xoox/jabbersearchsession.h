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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_JABBERSEARCHSESSION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_JABBERSEARCHSESSION_H
#include <QObject>
#include <interfaces/ihavesearch.h>
#include "jabbersearchmanager.h"

class QStandardItemModel;

namespace LeechCraft
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
		Q_INTERFACES (LeechCraft::Azoth::ISearchSession);

		GlooxAccount *Acc_;
		QStandardItemModel *Model_;

		JabberSearchManager *SM_;

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
