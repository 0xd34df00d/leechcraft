/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaType>

namespace LC::Azoth
{
	enum CLRoles
	{
		CLRAccountObject = Qt::UserRole + 1,
		CLREntryObject,
		CLRIEntry,
		CLREntryType,
		CLREntryCategory,
		CLRUnreadMsgCount,
		CLRRole,
		CLRAffiliation,
		CLRNumOnline,
		CLRIsMUCCategory,
	};

	enum CLEntryType
	{
		CLETAccount,
		CLETCategory,
		CLETContact,
	};
}

Q_DECLARE_METATYPE (LC::Azoth::CLEntryType)
