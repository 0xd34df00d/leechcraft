/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include <functional>
#include <QObject>
#include <QQueue>
#include <QDateTime>
#include <QUrl>
#include <QStringList>
#include <QVariant>

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class Account;

	struct DriveItem
	{
		enum ItemLabel
		{
			ILNone = 0x00,
			ILFavorite = 0x01,
			ILHidden = 0x02,
			ILRemoved = 0x04,
			ILShared = 0x08,
			ILViewed = 0x10
		};
		Q_DECLARE_FLAGS (ItemLabels, ItemLabel);

		enum class Roles
		{
			Owner,
			Writer,
			Reader
		};

		enum AdditionalRole
		{
			ARNone = 0x00,
			ARCommenter = 0x01
		};
		Q_DECLARE_FLAGS (AdditionalRoles, AdditionalRole);

		enum class PermissionTypes
		{
			User,
			Group,
			Domain,
			Anyone
		};

		QString Id_;

		QString ParentId_;
		bool ParentIsRoot_;

		QString Name_;
		QString OriginalFileName_;
		QString Md5_;
		QString Mime_;

		qint64 FileSize_;

		QStringList OwnerNames_;
		QString LastModifiedBy_;

		bool Editable_;
		bool WritersCanShare_;

		bool IsFolder_;

		ItemLabels Labels_;

		QDateTime CreateDate_;
		QDateTime ModifiedDate_;
		QDateTime LastViewedByMe_;

		QUrl DownloadUrl_;

		Roles PermissionRole_;
		AdditionalRoles PermissionAdditionalRole_;
		PermissionTypes PermissionType_;
	};

	class DriveManager : public QObject
	{
		Q_OBJECT

		Account *Account_;
		QQueue<std::function<void (const QString&)>> ApiCallQueue_;
	public:
		DriveManager (Account *acc, QObject *parent = 0);

		void RefreshListing ();

		void RequestFiles (const QString& key);
		void RequestFileShared (const QString& id, const QString& key);
	private:
		void RequestAccessToken ();
		void ParseError (const QVariantMap& map);

	private slots:
		void handleAuthTokenRequestFinished ();
		void handleGotFiles ();
		void handleRequestFileSharing ();

	signals:
		void gotFiles (const QList<DriveItem>& items);
	};
}
}
}
