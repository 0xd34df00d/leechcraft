/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QFileSystemModel>
#include <util/models/dndactionsmixin.h>
#include "localcollection.h"

namespace LC
{
namespace LMP
{
	class FSModel : public Util::DndActionsMixin<QFileSystemModel>
	{
		Q_OBJECT
	public:
		FSModel (QObject* = 0);
	};
}
}
