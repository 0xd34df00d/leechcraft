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

#ifndef UTIL_SYNCDELTAMANAGER_H
#define UTIL_SYNCDELTAMANAGER_H
#include <QObject>
#include <QDir>
#include <QSettings>
#include <interfaces/isyncable.h>
#include <util/utilconfig.h>

namespace LeechCraft
{
	namespace Util
	{
		class UTIL_API SyncDeltaManager : public QObject
		{
			Q_OBJECT

			QString ID_;
			QSettings Settings_;
		public:
			SyncDeltaManager (const QString&, QObject* = 0);
			virtual ~SyncDeltaManager ();

			void Store (const Sync::ChainID_t&, const Sync::Payload&);
			void Store (const Sync::ChainID_t&, const Sync::Payloads_t&);

			Sync::Payloads_t Get (const Sync::ChainID_t&);
			void Purge (const Sync::ChainID_t&, quint32 num);

			void DeltasRequested (const Sync::ChainID_t&);
		private:
			QDir GetDir (const Sync::ChainID_t&) const;
			int GetLastFileNum (const Sync::ChainID_t&);
			void SetLastFileNum (const Sync::ChainID_t&, int);
			void StoreImpl (const QString&, const Sync::Payload&);
		};
	}
}

#endif

