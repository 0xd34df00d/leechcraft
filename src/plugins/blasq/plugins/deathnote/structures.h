/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDateTime>
#include <QStringList>
#include <QUrl>

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	enum class Access
	{
		Private,
		Public,
		FriendsOnly,
		CustomUsers
	};

	struct Quota
	{
		quint64 Total_ = 0;
		quint64 Used_ = 0;
		quint64 Remaining_ = 0;
	};

	struct Album
	{
		QByteArray ID_;
		QString Title_;
		QDateTime CreationDate_;
		QUrl Url_;
		Access Access_;
	};

	struct Thumbnail
	{
		QUrl Url_;
		int Width_ = 0;
		int Height_ = 0;
	};

	struct Photo
	{
		QByteArray ID_;
		QString Title_;
		QString Format_;
		int Width_ = 0;
		int Height_ = 0;
		quint64 Size_ = 0;
		QByteArray MD5_;
		QUrl Url_;
		QString OriginalFileName_;
		QString Description_;
		Access Access_ = Access::Public;

		QList<Thumbnail> Thumbnails_;
	};
}
}
}
