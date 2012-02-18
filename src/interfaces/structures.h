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

#ifndef INTERFACES_STRUCTURES_H
#define INTERFACES_STRUCTURES_H
#include <memory>
#include <QPointer>
#include <QMetaType>
#include <QVariant>
#include <QString>
#include <QByteArray>
#include <QToolBar>
#include <QtDebug>
#include <util/idpool.h>

class QMenu;
class QNetworkReply;
class QIODevice;

namespace LeechCraft
{
	/** @brief Describes single task parameter.
	 */
	enum TaskParameter
	{
		/** Use default parameters.
		 */
		NoParameters = 0,

		/** Task should not be started automatically after addition.
		 */
		NoAutostart = 1,

		/** Task should not be saved in history.
		 */
		DoNotSaveInHistory = 2,

		/** Task is really downloaded, so, a file, for example, has
		 * appeared as a result.
		 */
		IsDownloaded = 4,

		/** Task is created as a result of user's actions.
		 */
		FromUserInitiated = 8,

		/** User should not be notified about task finish.
		 */
		DoNotNotifyUser = 32,

		/** Task is used internally and would not be visible to the user
		 * at all.
		 */
		Internal = 64,

		/** Task should not be saved as it would have no meaning after
		 * next start.
		 */
		NotPersistent = 128,

		/** When the task is finished, it should not be announced via
		 * gotEntity() signal.
		 */
		DoNotAnnounceEntity = 256,

		/** This task should not be downloaded, only handled by a
		 * handler.
		 */
		OnlyHandle = 512,

		/** This task should not be handled, only downloaded by a
		 * downloader.
		 */
		OnlyDownload = 1024,

		/** This task should be automatically accepted if any handler is
		 * available.
		 */
		AutoAccept = 2048,

		/** The plugin that was the source of this task should be
		 * queried if it could handle the task.
		 */
		ShouldQuerySource = 4096
	};

	Q_DECLARE_FLAGS (TaskParameters, TaskParameter);

	/** @brief Describes parameters of an entity.
	 *
	 * This struct is used both for addition of new download tasks and
	 * for announcing about finished/available entities, so its members
	 * are context-dependent.
	 *
	 * This is generally used for communications between different
	 * plugins. So, it can be thought as a general packet or message.
	 *
	 * There are two kinds of messages: notifications and delegation
	 * requests.
	 *
	 * First ones are asynchronous. They are used by plugins to notify
	 * Core and other plugins about events like download completion. In
	 * this case plugin that emits this notification doesn't care what
	 * happens next. For example, a bittorrent client that just finished
	 * downloading some files would emit a notification about those
	 * files.
	 *
	 * For notification messages the following signal is used:
	 * gotEntity (const LeechCraft::Entity& entity).
	 *
	 * The second type of messages, delegation requests, is used by
	 * plugins to delegate a given task to other plugins. For example,
	 * an RSS feed reader uses this kind of messages to delegate the
	 * downloading of the feeds via HTTP to a plugin that can handle
	 * HTTP. The signal that is used to emit this messages obviously
	 * blocks. After emitting the signal one could get the id and
	 * pointer to object that handles the request.
	 *
	 * For delegation requests the following signal is used:
	 * delegateEntity (const LeechCraft::Entity& entity, int *id, QObject **object);
	 *
	 * There is also a third signal related to messaging:
	 * couldHandle (const LeechCraft::Entity& entity, bool *could);
	 *
	 * It queries whether there are plugins that could handle the given
	 * entity. It also blocks. After emitting this signal the could
	 * variable would be set up accordingly.
	 *
	 * @sa LeechCraft::TaskParameter
	 */
	struct Entity
	{
		qint64 ID_;

		/** @brief The entity that this object represents.
		 *
		 * In the context of entity delegation it represents the entity
		 * that should be downloaded or handled. For example, contents
		 * of a torrent file, a magnet link, an RSS document.
		 *
		 * Here are some rules:
		 * - Local files should be a QUrl (QUrl::fromLocalFile).
		 * - URLs should be a QUrl as well.
		 * - Anything binary like contents of a torrent file should be a
		 *   QByteArray.
		 *
		 * In the context of announcing about a finished entity, it
		 * could contain previously mentioned entities as well.
		 */
		QVariant Entity_;

		/** @brief Source or destination.
		 *
		 * In the context of entity delegation this parameter represents
		 * where the other plugin wants this job to be downloaded.
		 *
		 * In the context of entity announce this represents from where
		 * this entity came - original URL, for example.
		 */
		QString Location_;

		/** @brief MIME type of the entity.
		 *
		 * An empty mime is considered to be no mime.
		 *
		 * Some predefined or commonly used ones:
		 * - x-leechcraft/notification
		 *   A notification item. It should have a "Priority"
		 *   Additional_ member with int values of enum Priority,
		 *   Entity_ is expected to be a QString with notification
		 *   header and Additional_ ["Text"] is expected to be the
		 *   notification text.
		 *   Additional_ ["NotificationActions"] may have
		 *   user-readable QStringList of actions to be available in
		 *   the notification, In this case,
		 *   Additional_ ["HandlingObject"] must be a QObject* pointing
		 *   to the handling object. This object must have the slot
		 *   notificationActionTriggered(int) which would be called with
		 *   the index of the selected action, if any. The index
		 *   corresponds to the position in NotificationActions list.
		 * - x-leechcraft/plain-text-document
		 *   A plaintext document. Entity_ contains the contents of the
		 *   document in this case, and Additional_ ["Language"] may
		 *   contain the language of this document (like C++/JS/whatever).
		 */
		QString Mime_;

		/** @brief Parameters of this task.
		 */
		TaskParameters Parameters_;

		/** @brief Additional parameters.
		 *
		 * Some predefined or commonly used ones:
		 * - "UserVisibleName"
		 *   A QString with some text that would make sense when showed
		 *   to the user. For example, if the entity is binary, this could
		 *   be used to describe the entity.
		 * - " Tags"
		 *   QStringList with IDs of tags of the entity.
		 */
		QMap<QString, QVariant> Additional_;

		Entity ()
		: ID_ (IDPool_.GetID ())
		{}
	protected:
		UTIL_API static Util::IDPool<qint64> IDPool_;
	};

	/** This enumeration describes the additional roles that may be
	 * returned from models that are embedded in Summary-like viewers.
	 * Those like IJobHolders or IFindProxies.
	 *
	 * @sa IJobHolder, IFindProxy
	 */
	enum CustomDataRoles
	{
		/** The role for the string list with tags. So, QStringList is
		 * expected to be returned.
		 */
		RoleTags = Qt::UserRole + 100,

		/** The role for the additional controls for a given item.
		 * QToolBar* is expected to be returned.
		 */
		RoleControls,

		/** The role for the widget appearing on the right part of the
		 * screen when the user selects an item. QWidget* is expected to
		 * be returned.
		 */
		RoleAdditionalInfo,

		/** The role for the hash of the item, used to compare two
		 * different results, possibly from two different models.
		 * QByteArray is expected to be returned.
		 */
		RoleHash,

		/** This should return MIME of an item if it's available,
		 * otherwise an empty string should be returned.
		 */
		RoleMime,

		/** This role returns the QMenu* that should be used as the
		 * context menu.
		 */
		RoleContextMenu,

		/** This role is for the LeechCraft::JobHolderRow enum.
		 */
		RoleJobHolderRow,

		RoleMAX
	};

	enum Priority
	{
		PLog_,
		PInfo_,
		PWarning_,
		PCritical_
	};
};

typedef std::shared_ptr<QObject> QObject_ptr;

Q_DECLARE_METATYPE (LeechCraft::Entity);
Q_DECLARE_METATYPE (QNetworkReply*);
Q_DECLARE_METATYPE (QIODevice*);
Q_DECLARE_METATYPE (QToolBar*);
Q_DECLARE_METATYPE (QMenu*);
Q_DECLARE_METATYPE (QObject_ptr);
Q_DECLARE_METATYPE (QPointer<QObject*>);
Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::TaskParameters);

#endif
