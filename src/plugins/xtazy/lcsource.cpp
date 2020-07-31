/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lcsource.h"
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace Xtazy
{
	LCSource::LCSource (QObject *parent)
	: TuneSourceBase { "LC", parent }
	{
	}

	void LCSource::NowPlaying (const Media::AudioInfo& audio)
	{
		EmitChange (audio);
	}

	void LCSource::Stopped ()
	{
		EmitChange (Media::AudioInfo ());
	}
}
}
