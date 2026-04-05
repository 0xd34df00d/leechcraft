/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC::Liznoo::Events
{
	class PlatformLayer : public QObject
	{
	protected:
		bool IsAvailable_ = false;
	public:
		using QObject::QObject;

		bool IsAvailable () const;

		void NotifyGonnaSleep (int);
		void NotifyWokeUp ();
	};
}
