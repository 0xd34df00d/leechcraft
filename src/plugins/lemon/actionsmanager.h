/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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
#include <QMap>
#include <interfaces/iactionsexporter.h>

class QAction;
class QNetworkConfigurationManager;
class QNetworkConfiguration;
class QNetworkSession;

typedef std::shared_ptr<QNetworkSession> QNetworkSession_ptr;

namespace LeechCraft
{
namespace Lemon
{
	class ActionsManager : public QObject
	{
		Q_OBJECT

		QNetworkConfigurationManager *Manager_;

		struct InterfaceInfo
		{
			QAction *Action_;
			QMap<QString, QNetworkSession_ptr> Sessions_;
			quint64 PrevRead_;
			quint64 PrevWritten_;

			InterfaceInfo ();
		};
		QMap<QString, InterfaceInfo> Infos_;
	public:
		ActionsManager (QObject* = 0);

		QList<QAction*> GetActions () const;
	private slots:
		void addConfiguration (const QNetworkConfiguration&);
	signals:
		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}
