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

#include "roomconfigwidget.h"
#include <QVBoxLayout>
#include <QXmppMucManager.h>
#include "roomclentry.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "roomhandler.h"
#include "formbuilder.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	RoomConfigWidget::RoomConfigWidget (RoomCLEntry *room, QWidget *widget)
	: QWidget (widget)
	, FormWidget_ (0)
	, FB_ (new FormBuilder)
	, Room_ (room)
	{
		setLayout (new QVBoxLayout);
		GlooxAccount *acc = qobject_cast<GlooxAccount*> (room->GetParentAccount ());
		QXmppMucManager *mgr = acc->GetClientConnection ()->GetMUCManager ();
		connect (mgr,
				SIGNAL (roomConfigurationReceived (const QString&, const QXmppDataForm&)),
				this,
				SLOT (handleRoomConfigurationReceived (const QString&, const QXmppDataForm&)));
		mgr->requestRoomConfiguration (room->GetRoomHandler ()->GetRoomJID ());
	}
	
	void RoomConfigWidget::handleRoomConfigurationReceived (const QString& jid, const QXmppDataForm& form)
	{
		if (jid != Room_->GetRoomHandler ()->GetRoomJID ())
			return;

		FormWidget_ = FB_->CreateForm (form);
		layout ()->addWidget (FormWidget_);
		emit dataReady ();
	}
}
}
}
