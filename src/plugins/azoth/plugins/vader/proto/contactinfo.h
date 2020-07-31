/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	struct ContactInfo
	{
		qint64 ContactID_;

		quint32 GroupNumber_;
		quint32 StatusID_;
		QString Email_;
		QString Phone_;
		QString Alias_;

		QString StatusTitle_;
		QString StatusDesc_;

		quint32 Features_;

		QString UA_;
	};
}
}
}
}
