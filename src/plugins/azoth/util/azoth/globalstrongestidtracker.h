/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/azothcommon.h>
#include "azothutilconfig.h"

namespace LC::Azoth
{
	class ICLEntry;

	class AZOTH_UTIL_API GlobalStrongestIdTracker : public QObject
	{
		GlobalStrongestId Id_;
	public:
		explicit GlobalStrongestIdTracker (ICLEntry& entry, QObject* = nullptr);

		GlobalStrongestId GetId () const;

		operator GlobalStrongestId () const;
	};

	QDebug operator<< (const QDebug&, const GlobalStrongestIdTracker&);
}
