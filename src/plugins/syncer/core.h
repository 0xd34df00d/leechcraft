/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_SYNCER_CORE_H
#define PLUGINS_SYNCER_CORE_H
#include <QObject>
#include <QSettings>
#include <interfaces/iinfo.h>
#include <interfaces/isyncable.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			class DataStorageBase;

			class Core : public QObject
			{
				Q_OBJECT

				DataStorageBase *DataStorage_;
				mutable QSettings Settings_;

				ICoreProxy_ptr Proxy_;

				QHash<QString, QObject*> ID2Object_;

				Core ();
			public:
				static Core& Instance ();
				void SetProxy (ICoreProxy_ptr);
				void SecondInit ();

				quint32 GetLastID (const QByteArray&) const;
				void SetLastID (const QByteArray&, quint32);
			private slots:
				void handleNewDeltas (const Sync::Deltas_t&, const QByteArray&);
				void handleDeltasRequired (Sync::Deltas_t*, const QByteArray&);
				void handleSuccessfullySentDeltas (quint32, const QByteArray&);
				void handleLoginError (const QByteArray&);
				void handleConnectionError (const QByteArray&);
				void handleFinishedSuccessfully (const QByteArray&);
			private:
				QString GetNameForChain (const QByteArray&);
			signals:
				void gotEntity (const LeechCraft::Entity&);
			};
		}
	}
}

#endif
