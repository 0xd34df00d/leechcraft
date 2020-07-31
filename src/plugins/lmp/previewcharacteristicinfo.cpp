/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "previewcharacteristicinfo.h"
#include <QHash>
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace LMP
{
	PreviewCharacteristicInfo::PreviewCharacteristicInfo (const Media::AudioInfo& info)
	: Artist_ { info.Album_.toLower ().trimmed () }
	, Title_ { info.Title_.toLower ().trimmed () }
	, Length_ { info.Length_ }
	{
	}

	bool operator== (const PreviewCharacteristicInfo& i1, const PreviewCharacteristicInfo& i2)
	{
		return i1.Artist_ == i2.Artist_ &&
				i1.Title_ == i2.Title_ &&
				i1.Length_ == i2.Length_;
	}

	uint qHash (const PreviewCharacteristicInfo& info)
	{
		return qHash (info.Artist_ + '|' + info.Title_ + '|' + QString::number (info.Length_));
	}
}
}
