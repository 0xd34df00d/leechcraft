/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QString>
#include <QObject>
#include "interfaces/azoth/isupportriex.h"

namespace LC
{
namespace Azoth
{
namespace RIEX
{
	void HandleRIEXItemsSuggested (QList<RIEXItem> items, QObject *from, QString message);
}
}
}
