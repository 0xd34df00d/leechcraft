/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tunesourcebase.h"
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace Xtazy
{
	TuneSourceBase::TuneSourceBase (const QByteArray& sourceName, QObject *parent)
	: QObject { parent }
	, SourceName_ { sourceName }
	{
	}

	const QByteArray& TuneSourceBase::GetSourceName () const
	{
		return SourceName_;
	}

	void TuneSourceBase::EmitChange (const Media::AudioInfo& info)
	{
		emit tuneInfoChanged (info, this);
	}

	Media::AudioInfo TuneSourceBase::FromMPRISMap (const QVariantMap& map)
	{
		return
		{
			map ["artist"].toString (),
			map ["source"].toString (),
			map ["title"].toString (),
			{},
			map ["length"].toInt (),
			0,
			map ["track"].toInt (),
			{}
		};
	}
}
}
