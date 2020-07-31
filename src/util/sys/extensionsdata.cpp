/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "extensionsdata.h"
#include <QIcon>
#include "extensionsdataimpl.h"

namespace LC
{
namespace Util
{
	ExtensionsData::ExtensionsData ()
#ifdef HAVE_EXTENSIONS_DATA
	: Impl_ { new ExtensionsDataImpl }
#else
	: Impl_ { nullptr }
#endif
	{
	}

	ExtensionsData& ExtensionsData::Instance ()
	{
		static ExtensionsData ed;
		return ed;
	}

	QString ExtensionsData::GetMime (const QString& extension) const
	{
#ifdef HAVE_EXTENSIONS_DATA
		return Impl_ ? Impl_->GetMimeDatabase () [extension] : QString {};
#else
		return {};
#endif
	}

	QIcon ExtensionsData::GetExtIcon (const QString& extension) const
	{
#ifdef HAVE_EXTENSIONS_DATA
		return Impl_ ? Impl_->GetExtIcon (extension) : QIcon {};
#else
		return {};
#endif
	}

	QIcon ExtensionsData::GetMimeIcon (const QString& mime) const
	{
#ifdef HAVE_EXTENSIONS_DATA
		return Impl_ ? Impl_->GetMimeIcon (mime) : QIcon {};
#else
		return {};
#endif
	}
}
}
