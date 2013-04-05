/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
namespace AN
{
	/** @brief Category of Instant Messaging-related events.
	 */
	const QString CatIM = "org.LC.AdvNotifications.IM";
	/** @brief Another user has requested our user's attention.
	 */
	const QString TypeIMAttention = "org.LC.AdvNotifications.IM.AttentionDrawn";
	/** @brief Another user has sent our user a file.
	 */
	const QString TypeIMIncFile = "org.LC.AdvNotifications.IM.IncomingFile";
	/** @brief User has received a message in a standard one-to-one chat.
	 */
	const QString TypeIMIncMsg = "org.LC.AdvNotifications.IM.IncomingMessage";
	/** @brief User has been highlighted in a multiuser chat.
	 *
	 * The primary difference from TypeIMMUCMsg is that our user must be
	 * explicitly mentioned in another user's message for this event.
	 *
	 * @sa TypeIMMUCMsg
	 */
	const QString TypeIMMUCHighlight = "org.LC.AdvNotifications.IM.MUCHighlightMessage";
	/** @brief User has been invited to a multiuser chat.
	 */
	const QString TypeIMMUCInvite = "org.LC.AdvNotifications.IM.MUCInvitation";
	/** @brief A message has been sent to a multiuser chat.
	 *
	 * This event should be emitted for each MUC message, even for those
	 * our user isn't mentioned in.
	 *
	 * @sa TypeIMMUCHighlight
	 */
	const QString TypeIMMUCMsg = "org.LC.AdvNotifications.IM.MUCMessage";
	/** @brief Another user in our user's contact list has changed its
	 * status.
	 */
	const QString TypeIMStatusChange = "org.LC.AdvNotifications.IM.StatusChange";
	/** @brief Another user has granted subscription to our user.
	 */
	const QString TypeIMSubscrGrant = "org.LC.AdvNotifications.IM.Subscr.Granted";
	/** @brief Another user has revoked subscription from our user.
	 */
	const QString TypeIMSubscrRevoke = "org.LC.AdvNotifications.IM.Subscr.Revoked";
	/** @brief Another user has requested subscription from our user.
	 */
	const QString TypeIMSubscrRequest = "org.LC.AdvNotifications.IM.Subscr.Requested";
	/** @brief Another user has subscribed to our user.
	 */
	const QString TypeIMSubscrSub = "org.LC.AdvNotifications.IM.Subscr.Subscribed";
	/** @brief Another user has unsubscribed from our user.
	 */
	const QString TypeIMSubscrUnsub = "org.LC.AdvNotifications.IM.Subscr.Unsubscribed";

	/** @brief Category of Organizer-related events.
	 */
	const QString CatOrganizer = "org.LC.AdvNotifications.Organizer";
	/** @brief An event due date is coming.
	 */
	const QString TypeOrganizerEventDue = "org.LC.AdvNotifications.Organizer.EventDue";

	/** @brief Category of Downloads-related events.
	 */
	const QString CatDownloads = "org.LC.AdvNotifications.Downloads";
	/** @brief A download has been finished successfully without errors.
	 */
	const QString TypeDownloadFinished = "org.LC.AdvNotifications.Downloads.DownloadFinished";
	/** @brief A download has been failed.
	 */
	const QString TypeDownloadError = "org.LC.AdvNotifications.Downloads.DownloadError";
}
}
