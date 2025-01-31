/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <interfaces/media/audiostructs.h>
#include "lmputilconfig.h"

namespace LC::LMP
{
	struct MediaInfo
	{
		QString LocalPath_ {};

		QString Artist_ {};
		QString Album_ {};
		QString Title_ {};

		QStringList Genres_ {};

		qint32 Length_ = 0;
		qint32 Year_ = 0;
		qint32 TrackNumber_ = 0;

		QVariantMap Additional_ {};

		LMP_UTIL_API bool IsUseless () const;

		LMP_UTIL_API operator Media::AudioInfo () const;

		LMP_UTIL_API static MediaInfo FromAudioInfo (const Media::AudioInfo&);
	};

	LMP_UTIL_API bool operator== (const MediaInfo& l, const MediaInfo& r);
	LMP_UTIL_API bool operator!= (const MediaInfo& l, const MediaInfo& r);

	LMP_UTIL_API QDataStream& operator<< (QDataStream&, const MediaInfo&);
	LMP_UTIL_API QDataStream& operator>> (QDataStream&, MediaInfo&);
}

Q_DECLARE_METATYPE (LC::LMP::MediaInfo)
