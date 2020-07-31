/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QString>
#include "structures.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	QString FormatUserInfoName (const UserInfo& info)
	{
		auto string = XmlSettingsManager::Instance ().property ("EntryNameFormat").toString ();
		string.replace ("$name", info.FirstName_);
		string.replace ("$surname", info.LastName_);
		string.replace ("$nick", info.Nick_);
		string.replace ("  ", " ");
		return string;
	}
}
}
}
