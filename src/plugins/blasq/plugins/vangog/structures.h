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
namespace Vangog
{
	enum class Access
	{
		Private,
		Public
	};

	struct Author
	{
		QString Name_;
		QUrl Image_;
	};

	struct Thumbnail
	{
		QUrl Url_;
		int Width_;
		int Height_;

		Thumbnail ()
		: Width_ (0)
		, Height_ (0)
		{}
	};

	struct Album
	{
		QByteArray ID_;
		QString Title_;
		QString Description_;
		QDateTime Published_;
		QDateTime Updated_;
		Access Access_;
		Author Author_;
		int NumberOfPhoto_;
		quint64 BytesUsed_;
		QList<Thumbnail> Thumbnails_;

		Album ()
		: Access_ (Access::Private)
		, NumberOfPhoto_ (0)
		, BytesUsed_ (0)
		{}
	};

	struct Exif
	{
		QString Manufacturer_;
		QString Model_;
		int FNumber_;
		float Exposure_;
		bool Flash_;
		float FocalLength_;
		int ISO_;

		Exif ()
		: FNumber_ (0)
		, Exposure_ (0.0)
		, Flash_ (false)
		, FocalLength_ (0.0)
		, ISO_ (0)
		{}
	};

	struct Photo
	{
		QByteArray ID_;
		QString Title_;
		QDateTime Published_;
		QDateTime Updated_;
		Access Access_;
		QByteArray AlbumID_;
		int Width_;
		int Height_;
		quint64 Size_;
		Exif Exif_;
		QUrl Url_;
		QStringList Tags_;
		QList<Thumbnail> Thumbnails_;

		Photo ()
		: Access_ (Access::Private)
		, Width_ (0)
		, Height_ (0)
		, Size_ (0)
		{}
	};
}
}
}
