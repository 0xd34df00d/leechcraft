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

#pragma once

#include <QString>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	const QString CatIM = "org.LC.AdvNotifications.IM";
	const QString TypeIMAttention = "org.LC.AdvNotifications.IM.AttentionDrawn";
	const QString TypeIMIncFile = "org.LC.AdvNotifications.IM.IncomingFile";
	const QString TypeIMIncMsg = "org.LC.AdvNotifications.IM.IncomingMessage";
	const QString TypeIMMUCHighlight = "org.LC.AdvNotifications.IM.MUCHighlightMessage";
	const QString TypeIMMUCInvite = "org.LC.AdvNotifications.IM.MUCInvitation";
	const QString TypeIMMUCMsg = "org.LC.AdvNotifications.IM.MUCMessage";
	const QString TypeIMStatusChange = "org.LC.AdvNotifications.IM.StatusChange";
	const QString TypeIMSubscrGrant = "org.LC.AdvNotifications.IM.Subscr.Granted";
	const QString TypeIMSubscrRevoke = "org.LC.AdvNotifications.IM.Subscr.Revoked";
	const QString TypeIMSubscrRequest = "org.LC.AdvNotifications.IM.Subscr.Requested";
	const QString TypeIMSubscrSub = "org.LC.AdvNotifications.IM.Subscr.Subscribed";
	const QString TypeIMSubscrUnsub = "org.LC.AdvNotifications.IM.Subscr.Unsubscribed";

	const QString CatOrganizer = "org.LC.AdvNotifications.Organizer";
	const QString TypeOrganizerEventDue = "org.LC.AdvNotifications.Organizer.EventDue";
}
}
