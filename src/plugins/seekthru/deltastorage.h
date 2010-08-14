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

#ifndef PLUGINS_SEEKTHRU_DELTASTORAGE_H
#define PLUGINS_SEEKTHRU_DELTASTORAGE_H
#include <QObject>
#include <QDir>
#include <interfaces/isyncable.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class DeltaStorage : public QObject
			{
				Q_OBJECT

				QString ID_;
			public:
				DeltaStorage (const QString&, QObject* = 0);

				void Store (const Sync::ChainID_t&, const Sync::Payload&);
				void Store (const Sync::ChainID_t&, const Sync::Payloads_t&);

				Sync::Payloads_t Get (const Sync::ChainID_t&) const;
				void Purge (const Sync::ChainID_t&, quint32 num);
			private:
				QDir GetDir (const Sync::ChainID_t&) const;
				int GetLastFileNum (const Sync::ChainID_t&) const;
				void SetLastFileNum (const Sync::ChainID_t&, int) const;
				void StoreImpl (const QString&, const Sync::Payload&);
			};
		}
	}
}

#endif
