/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "loadprocess.h"

namespace LC
{
	LoadProcess::LoadProcess (const QString& title, int min, int max)
	: Title_ { title }
	, Min_ { min }
	, Max_ { max }
	{
		LastReport_.start ();
	}

	QString LoadProcess::GetTitle () const
	{
		return Title_;
	}

	int LoadProcess::GetMin () const
	{
		return Min_;
	}

	int LoadProcess::GetMax () const
	{
		return Max_;
	}

	int LoadProcess::GetValue () const
	{
		return Value_;
	}

	void LoadProcess::ReportValue (int value)
	{
		if (Value_ == value)
			return;

		Value_ = value;

		if (LastReport_.elapsed () * 60 > 1000)
		{
			emit changed ();
			LastReport_.start ();
		}
	}
}
