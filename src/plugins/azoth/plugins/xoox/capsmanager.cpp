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

#include "capsmanager.h"
#include "clientconnection.h"
#include "capsdatabase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	CapsManager::CapsManager (ClientConnection *connection)
	: QObject (connection)
	, Connection_ (connection)
	, DB_ (new CapsDatabase (this))
	{
	}
	
	void CapsManager::FetchCaps (const QString& jid, const QByteArray& verNode)
	{
		if (!DB_->Contains (verNode))
			Connection_->RequestInfo (jid);
	}
	
	QStringList CapsManager::GetRawCaps (const QByteArray& verNode) const
	{
		return DB_->Get (verNode);
	}
	
	QStringList CapsManager::GetCaps (const QByteArray& verNode) const
	{
		QStringList result;
		Q_FOREACH (const QString& raw, GetRawCaps (verNode))
			result << Caps2String_.value (raw, raw);
		return result;
	}

	void CapsManager::handleInfoReceived (const QXmppDiscoveryIq& iq)
	{
		if (!iq.features ().isEmpty ())
			DB_->Set (iq.verificationString (), iq.features ());
	}
	
	void CapsManager::handleItemsReceived (const QXmppDiscoveryIq& iq)
	{
		if (!iq.features ().isEmpty ())
			DB_->Set (iq.verificationString (), iq.features ());
	}
}
}
}
