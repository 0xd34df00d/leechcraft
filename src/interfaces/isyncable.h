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

#ifndef INTERFACES_ISYNCABLE_H
#define INTERFACES_ISYNCABLE_H
#include <QByteArray>
#include <QSet>
#include <QList>
#include <QMetaType>

namespace LeechCraft
{
	namespace Sync
	{
		struct Payload
		{
			QByteArray Data_;
		};

		typedef QList<Payload> Payloads_t;

		struct Delta
		{
			quint64 ID_;
			Payload Payload_;
		};

		typedef QList<Delta> Deltas_t;

		typedef QByteArray ChainID_t;

		typedef QSet<ChainID_t> ChainIDs_t;
	}
}

Q_DECLARE_METATYPE (LeechCraft::Sync::ChainID_t);

/** @brief Interface for plugins that have content/data/settings that
 * can be synchronized via other plugins — syncers.
 *
 * To notify about new deltas, the following signal is expected:
 * newDeltasAvailable(const ChainID_t& chain)
 */
class ISyncable
{
public:
	virtual ~ISyncable () {}

	virtual LeechCraft::Sync::ChainIDs_t AvailableChains () const = 0;

	virtual LeechCraft::Sync::Payloads_t GetAllDeltas (const LeechCraft::Sync::ChainID_t& chain) const = 0;

	virtual LeechCraft::Sync::Payloads_t GetNewDeltas (const LeechCraft::Sync::ChainID_t& chain) const = 0;

	virtual void PurgeNewDeltas (const LeechCraft::Sync::ChainID_t& chain) = 0;

	virtual void ApplyDeltas (const LeechCraft::Sync::Payloads_t& deltas,
			const LeechCraft::Sync::ChainID_t& chain) = 0;
};

Q_DECLARE_INTERFACE (ISyncable, "org.Deviant.LeechCraft.Sync.ISyncable/1.0");

#endif
