/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "types.h"
#include <QFileInfo>
#include <util/sll/visitor.h>

namespace LC::Azoth::Transfers
{
	QString GetFilename (const JobContext& context)
	{
		return Util::Visit (context.Dir_,
				[] (JobContext::In in) { return QFileInfo { in.SavePath_ }.fileName (); },
				[&] (JobContext::Out) { return context.OrigFilename_; });
	}
}
