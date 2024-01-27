/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "document.h"
#include <util/monocle/resourcedtextdocument.h>

namespace LC::Monocle::Boop
{
	Document::Document (QUrl url, QObject *pluginObj)
	: PluginObject_ { pluginObj }
	, Url_ { std::move (url) }
	{
	}

	QObject* Document::GetBackendPlugin () const
	{
		return PluginObject_;
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		return {};
	}

	QUrl Document::GetDocURL () const
	{
		return Url_;
	}
}
