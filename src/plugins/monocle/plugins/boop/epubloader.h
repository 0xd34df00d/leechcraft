/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/monocle/idocument.h>

class QString;
class QObject;

namespace LC::Monocle
{
	using IDocument_ptr = std::shared_ptr<IDocument>;
}

namespace LC::Monocle::Boop
{
	IDocument_ptr LoadZip (const QString& file, QObject *pluginObj);
}
