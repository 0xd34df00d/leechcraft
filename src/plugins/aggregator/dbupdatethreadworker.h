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

#ifndef PLUGINS_AGGREGATOR_DBUPDATETHREADWORKER_H
#define PLUGINS_AGGREGATOR_DBUPDATETHREADWORKER_H
#include <memory>
#include <QObject>
#include <QVariantList>
#include <interfaces/core/ihookproxy.h>
#include "common.h"
#include "channel.h"

namespace LeechCraft
{
struct Entity;

namespace Aggregator
{
	class StorageBackend;

	class DBUpdateThreadWorker : public QObject
	{
		Q_OBJECT

		std::shared_ptr<StorageBackend> SB_;
	public:
		DBUpdateThreadWorker (QObject* = 0);
	public slots:
		void toggleChannelUnread (IDType_t channel, bool state);
		void updateFeed (channels_container_t channels, QString url);
	private slots:
		void handleChannelDataUpdated (Channel_ptr);
	signals:
		void channelDataUpdated (IDType_t channelId, IDType_t feedId);
		void gotNewChannel (const ChannelShort&);
		void gotEntity (const LeechCraft::Entity&);
		void itemDataUpdated (Item_ptr, Channel_ptr);

		void hookGotNewItems (LeechCraft::IHookProxy_ptr proxy,
				QVariantList items);
	};
}
}

#endif
