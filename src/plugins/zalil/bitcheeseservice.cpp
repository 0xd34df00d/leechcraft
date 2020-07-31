/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bitcheeseservice.h"
#include "bitcheesepending.h"

namespace LC
{
namespace Zalil
{
	QString BitcheeseService::GetName () const
	{
		return "dump.bitcheese.net";
	}

	qint64 BitcheeseService::GetMaxFileSize () const
	{
		return 41 * 1024 * 1024;
	}

	PendingUploadBase* BitcheeseService::UploadFile (const QString& file)
	{
		return new BitcheesePending { file, Proxy_, this };
	}
}
}
