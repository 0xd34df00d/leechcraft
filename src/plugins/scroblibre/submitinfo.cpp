/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "submitinfo.h"

namespace LC
{
namespace Scroblibre
{
	SubmitInfo::SubmitInfo (const Media::AudioInfo& info)
	: Info_ (info)
	, TS_ (QDateTime::currentDateTime ())
	{
	}

	SubmitInfo::SubmitInfo (const Media::AudioInfo& info, const QDateTime& ts)
	: Info_ (info)
	, TS_ (ts)
	{
	}

	SubmitInfo& SubmitInfo::operator= (const Media::AudioInfo& info)
	{
		Info_ = info;
		TS_ = QDateTime::currentDateTime ();
		return *this;
	}

	void SubmitInfo::Clear ()
	{
		Info_ = Media::AudioInfo ();
		TS_ = QDateTime ();
	}

	bool SubmitInfo::IsValid () const
	{
		return TS_.isValid ();
	}
}
}
