/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/azoth/azothcommon.h>

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace VaderUtil
{
	State StatusID2State (quint32);
	quint32 State2StatusID (State);
	QList<QAction*> GetBuddyServices (QObject *receiver, const char *slot);
	QString SubstituteNameDomain (const QString& string, const QString& fullEmail);
}
}
}
}
