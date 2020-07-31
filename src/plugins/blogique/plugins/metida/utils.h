/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include "entryoptions.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
namespace MetidaUtils
{
	QString GetLocalizedErrorMessage (int errorCode);

	QString GetStringForAccess (Access access);
	Access GetAccessForString (const QString& access);

	QString GetStringForAdultContent (AdultContent adult);
	AdultContent GetAdultContentFromString (const QString& str);

	CommentsManagement GetCommentsManagmentFromString (const QString& str);
	CommentsManagement GetCommentsManagmentFromInt (int cm);
	QString GetStringFromCommentsManagment (CommentsManagement cm);
}
}
}
}

