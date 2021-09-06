/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QMetaType>
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace LMP
{
	struct MediaInfo
	{
		QString LocalPath_ = {};

		QString Artist_ = {};
		QString Album_ = {};
		QString Title_ = {};

		QStringList Genres_ = {};

		qint32 Length_ = 0;
		qint32 Year_ = 0;
		qint32 TrackNumber_ = 0;

		QVariantMap Additional_ = {};

		bool IsUseless () const;

		operator Media::AudioInfo () const;

		static MediaInfo FromAudioInfo (const Media::AudioInfo&);
	};

	inline bool operator== (const MediaInfo& l, const MediaInfo& r)
	{
		return l.LocalPath_ == r.LocalPath_ &&
			l.Artist_ == r.Artist_ &&
			l.Album_ == r.Album_ &&
			l.Title_ == r.Title_ &&
			l.Genres_ == r.Genres_ &&
			l.Length_ == r.Length_ &&
			l.Year_ == r.Year_ &&
			l.TrackNumber_ == r.TrackNumber_;
	}

	inline bool operator!= (const MediaInfo& l, const MediaInfo& r)
	{
		return !(l == r);
	}

	QDataStream& operator<< (QDataStream&, const MediaInfo&);
	QDataStream& operator>> (QDataStream&, MediaInfo&);
}
}

Q_DECLARE_METATYPE (LC::LMP::MediaInfo)
