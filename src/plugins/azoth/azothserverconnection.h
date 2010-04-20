/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_AZOTHSERVERCONNECTION_H
#define PLUGINS_AZOTH_AZOTHSERVERCONNECTION_H
#include <memory>
#include <QObject>
#include <QDBusInterface>
#include <QProcess>
#include <interfaces/structures.h>

class QDBusPendingCallWatcher;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			class AzothServerConnection : public QObject
			{
				Q_OBJECT

				std::auto_ptr<QDBusInterface> Connection_;
			public:
				AzothServerConnection (QObject* = 0);

				void Establish ();
				void Release ();
				void ReaddProtocolPlugins ();
				void ServerReady ();
			private slots:
				void handleProcessError (QProcess::ProcessError);
				void handleAddProtocolPluginCallFinished (QDBusPendingCallWatcher*);
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};


#endif

