/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include <QMetaType>
#include <QVariant>
#include <QString>
#include <QByteArray>
#include <QToolBar>
#include <QtDebug>

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
	 * This struct is used both for addition of new download jobs and
	 * for announcing about finished/available entities, so its members
	 * are context-dependent.
	 *
	 * @sa LeechCraft::TaskParameter
	 */
	struct DownloadEntity
	{
		/** @brief The entity that this object represents.
		 *
		 * In the context of entity delegation it represents the entity
		 * that should be downloaded or handled. For example, contents
		 * of a torrent file, a magnet link, an RSS document.
		 *
		 * Local files should start with file://
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
	};

	enum CustomDataRoles
	{
		/** The role for the string list with tags. So, QStringList is
		 * expected to be returned.
		 */
		RoleTags = 100,
		/* The role for the additional controls for a given item.
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
		RoleContextMenu
	};

	struct Notification
	{
		enum Priority
		{
			PLog_,
			PInformation_,
			PWarning_,
			PCritical_
		};

		QString Header_;
		QString Text_;
		bool UntilUserSees_;
		Priority Priority_;
	};
};

Q_DECLARE_METATYPE (LeechCraft::DownloadEntity);
Q_DECLARE_METATYPE (LeechCraft::Notification);
Q_DECLARE_METATYPE (QNetworkReply*);
Q_DECLARE_METATYPE (QIODevice*);
Q_DECLARE_METATYPE (QToolBar*);
Q_DECLARE_METATYPE (QMenu*);
Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::TaskParameters);

#endif

