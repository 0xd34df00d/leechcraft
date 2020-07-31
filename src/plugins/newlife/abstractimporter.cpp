/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "abstractimporter.h"
#include <algorithm>

namespace LC
{
namespace NewLife
{
	QStringList AbstractImporter::GetIcons () const
	{
		QStringList result;
		std::fill_n (std::back_inserter (result), GetNames ().size (), QString {});
		return result;
	}
}
}
