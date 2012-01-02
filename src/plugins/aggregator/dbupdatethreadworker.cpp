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

#include "dbupdatethreadworker.h"
#include <stdexcept>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "storagebackend.h"

namespace LeechCraft
{
namespace Aggregator
{
	DBUpdateThreadWorker::DBUpdateThreadWorker (QObject *parent)
	: QObject (parent)
	{
		try
		{
			const QString& strType = XmlSettingsManager::Instance ()->
					property ("StorageType").toString ();
			SB_ = StorageBackend::Create (strType, "_UpdateThread");
		}
		catch (const std::runtime_error& s)
		{
			qWarning () << Q_FUNC_INFO
					<< s.what ();
			return;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown exception";
			return;
		}

		SB_->Prepare ();

		connect (SB_.get (),
				SIGNAL (channelDataUpdated (Channel_ptr)),
				this,
				SLOT (handleChannelDataUpdated (Channel_ptr)));
	}

	void DBUpdateThreadWorker::toggleChannelUnread (IDType_t channel, bool state)
	{
		SB_->ToggleChannelUnread (channel, state);
	}

	void DBUpdateThreadWorker::handleChannelDataUpdated (Channel_ptr ch)
	{
		emit channelDataUpdated (ch->ChannelID_, ch->FeedID_);
	}
}
}
