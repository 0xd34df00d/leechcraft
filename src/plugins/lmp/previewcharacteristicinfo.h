/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace Media
{
	struct AudioInfo;
}

namespace LC
{
namespace LMP
{
	struct PreviewCharacteristicInfo
	{
		QString Artist_;
		QString Title_;
		qint32 Length_;

		PreviewCharacteristicInfo (const Media::AudioInfo&);
	};

	bool operator== (const PreviewCharacteristicInfo&, const PreviewCharacteristicInfo&);
	uint qHash (const PreviewCharacteristicInfo&);
}
}
