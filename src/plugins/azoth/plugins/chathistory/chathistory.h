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

#ifndef PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CHATHISTORY_H
#define PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CHATHISTORY_H
#include <QObject>
#include <QSqlQuery>
#include <QSet>
#include <boost/shared_ptr.hpp>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>

class QSqlDatabase;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace ChatHistory
				{
					class Plugin : public QObject
								 , public IInfo
								 , public IPlugin2
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IPlugin2)

						boost::shared_ptr<QSqlDatabase> DB_;
						QSqlQuery UserSelecter_;
						QSqlQuery UserIdSelecter_;
						QSqlQuery UserInserter_;
						QSqlQuery MessageDumper_;
						QSet<QString> Users_;
						void InitializeTables ();
					public:
						void Init (ICoreProxy_ptr);
						void SecondInit ();
						QByteArray GetUniqueID () const;
						void Release ();
						QString GetName () const;
						QString GetInfo () const;
						QIcon GetIcon () const;
						QStringList Provides () const;
						QStringList Needs () const;
						QStringList Uses () const;
						void SetProvider (QObject*, const QString&);

						QSet<QByteArray> GetPluginClasses () const;
						QSqlQuery GetQuery (const QString& sql);
					public slots:
						void hookMessageCreated (LeechCraft::IHookProxy_ptr proxy,
								QObject *chatTab,
								QObject *message);
						void hookMessageArrived (LeechCraft::IHookProxy_ptr proxy,
								QObject *chatTab,
								QObject *message);
						void hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
													 QObject *message);
					};
				}
			}
		}
	}
}

#endif

