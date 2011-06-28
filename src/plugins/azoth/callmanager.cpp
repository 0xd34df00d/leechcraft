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

#include "callmanager.h"
#include <QtDebug>
#include "interfaces/imediacall.h"
#include "interfaces/iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	CallManager::CallManager (QObject *parent)
	: QObject (parent)
	{
	}
	
	void CallManager::AddAccount (QObject *account)
	{
		if (!qobject_cast<ISupportMediaCalls*> (account))
			return;
		
		connect (account,
				SIGNAL (called (QObject*)),
				this,
				SLOT (handleIncomingCall (QObject*)));
	}
	
	QObject* CallManager::Call (ICLEntry *entry, const QString& variant)
	{
		ISupportMediaCalls *ismc = qobject_cast<ISupportMediaCalls*> (entry->GetParentAccount ());
		if (!ismc)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetObject ()
					<< "parent account doesn't support media calls";
			return 0;
		}
		
		QObject *callObj = ismc->Call (entry->GetEntryID (), variant);
		HandleCall (callObj);
		return callObj;
	}
	
	QObjectList CallManager::GetCallsForEntry (const QString& id) const
	{
		return Entry2Calls_ [id];
	}
	
	void CallManager::HandleCall (QObject *obj)
	{
		IMediaCall *mediaCall = qobject_cast<IMediaCall*> (obj);
		if (!mediaCall)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "is not a IMediaCall, got from"
					<< sender ();
			return;
		}

		Entry2Calls_ [mediaCall->GetSourceID ()] << obj;
	}
	
	void CallManager::handleIncomingCall (QObject *obj)
	{
		HandleCall (obj);
		
		emit gotCall (obj);
	}
}
}
