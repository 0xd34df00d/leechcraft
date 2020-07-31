/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QList>
#include <QUrl>
#include <QDateTime>
#include <QVariantMap>
#include <QSize>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	struct ListInfo
	{
		qulonglong ID_ = 0;
		QString Name_;
	};

	struct AppInfo
	{
		qulonglong AppId_ = 0;

		bool IsMobile_ = false;

		QString Title_;
		QUrl Icon25_;
	};

	bool operator== (const AppInfo& left, const AppInfo& right);

	struct UserInfo
	{
		qulonglong ID_ = 0;

		QString FirstName_;
		QString LastName_;
		QString Nick_;

		QUrl Photo_;
		QUrl BigPhoto_;

		int Gender_ = 0;

		QDate Birthday_;

		QString HomePhone_;
		QString MobilePhone_;

		int Timezone_ = 0;

		int Country_ = 0;
		QString CountryName_;
		int City_ = 0;
		QString CityName_;

		bool IsOnline_ = false;

		QList<qulonglong> Lists_;

		AppInfo AppInfo_;

		static UserInfo FromID (qulonglong);
	};

	enum MessageFlag
	{
		None =			0,
		Unread =		1 << 0,
		Outbox = 		1 << 1,
		Replied = 		1 << 2,
		Important = 	1 << 3,
		/** This doesn't really mean the same as the VK docs say on the
		 * http://vk.com/dev/using_longpoll page. Instead, Chat flag
		 * means that the message relates to a multiuser chat.
		 */
		Chat = 			1 << 4,
		Friends = 		1 << 5,
		Spam = 			1 << 6,
		Deleted = 		1 << 7,
		Fixed = 		1 << 8,
		Media = 		1 << 9
	};

	Q_DECLARE_FLAGS (MessageFlags, MessageFlag)

	struct MessageInfo
	{
		qulonglong ID_ = 0;
		qulonglong From_ = 0;

		QString Text_;

		MessageFlags Flags_;

		QDateTime TS_;

		QVariantMap Params_;
	};

	struct ChatInfo
	{
		qulonglong ChatID_ = 0;

		QString Title_;
		QList<UserInfo> Users_;
	};

	enum class GeoIdType
	{
		Country,
		City
	};

	struct PhotoInfo
	{
		qlonglong OwnerID_ = 0;
		qulonglong ID_ = 0;
		qlonglong AlbumID_ = 0;

		QString Thumbnail_;
		QSize ThumbnailSize_;
		QString Full_;
		QSize FullSize_;

		QString AccessKey_;
	};

	struct AudioInfo
	{
		qlonglong OwnerID_ = 0;
		qulonglong ID_ = 0;

		QString Artist_;
		QString Title_;

		int Duration_ = 0;

		QUrl URL_;
	};

	struct VideoInfo
	{
		qlonglong OwnerID_ = 0;
		qulonglong ID_ = 0;

		QString AccessKey_;

		QString Title_;
		QString Desc_;
		qulonglong Duration_ = 0;

		qlonglong Views_ = 0;

		QUrl Image_;
	};

	struct DocumentInfo
	{
		qlonglong OwnerID_ = 0;
		qulonglong ID_ = 0;

		QString Title_;
		QString Extension_;

		qulonglong Size_ = 0;

		QUrl Url_;
	};

	struct PagePreview
	{
		qlonglong OwnerID_ = 0;
		qulonglong ID_ = 0;

		QString Title_;
		QUrl Url_;
	};

	struct GiftInfo
	{
		qulonglong Id_ = 0;
		QUrl Thumb_;
	};

	struct StickerInfo
	{
		QString Id_;
	};

	struct FullMessageInfo
	{
		qlonglong OwnerID_ = 0;
		qulonglong ID_ = 0;

		QString Text_;

		QDateTime PostDate_;
		int Likes_ = 0;
		int Reposts_ = 0;

		QList<PhotoInfo> Photos_;
		QList<AudioInfo> Audios_;
		QList<VideoInfo> Videos_;
		QList<DocumentInfo> Documents_;
		QList<GiftInfo> Gifts_;
		QList<StickerInfo> Stickers_;
		QList<PagePreview> PagesPreviews_;
		QList<FullMessageInfo> ContainedReposts_;
		QList<FullMessageInfo> ForwardedMessages_;
	};
}
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::Azoth::Murm::MessageFlags)
