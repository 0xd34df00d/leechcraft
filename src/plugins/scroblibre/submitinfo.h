/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDateTime>
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace Scroblibre
{
	struct SubmitInfo
	{
		Media::AudioInfo Info_;
		QDateTime TS_;

		SubmitInfo () = default;
		SubmitInfo (const Media::AudioInfo&);
		SubmitInfo (const Media::AudioInfo&, const QDateTime&);

		SubmitInfo& operator= (const Media::AudioInfo&);

		void Clear ();

		bool IsValid () const;
	};
}
}
