/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QMetaType>
#include <QVariant>
#include <QString>

class QMenu;
class QIODevice;

namespace LC
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

		/** The entity was generated from command line parameters or
		 * LeechCraft process invocation.
		 */
		FromCommandLine = 4096
	};

	Q_DECLARE_FLAGS (TaskParameters, TaskParameter);

	/** @brief A message used for inter-plugin communication.
	 *
	 * The IEntityManager class is responsible for routing entities between plugins.
	 *
	 * @sa IEntityManager
	 * @sa LC::TaskParameter
	 */
	struct Entity
	{
		/** @brief The entity that this object represents.
		 *
		 * In the context of entity delegation it represents the entity
		 * that should be downloaded or handled. For example, contents
		 * of a torrent file, a magnet link, an RSS document.
		 *
		 * Here are some common conventions:
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

		Entity () {}
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

		/** This role is for the LC::JobHolderRow enum.
		 */
		RoleJobHolderRow,

		RoleMAX
	};

	enum class Priority
	{
		Info,
		Warning,
		Critical
	};
};

typedef std::shared_ptr<QObject> QObject_ptr;

Q_DECLARE_METATYPE (LC::Entity)
Q_DECLARE_METATYPE (QObject_ptr)
Q_DECLARE_METATYPE (LC::TaskParameters)
Q_DECLARE_METATYPE (LC::Priority)
Q_DECLARE_OPERATORS_FOR_FLAGS (LC::TaskParameters)
