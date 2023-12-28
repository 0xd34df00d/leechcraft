/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "document.h"
#include <QTextDocument>

namespace LC::Monocle::Boop
{
	Document::Document (std::unique_ptr<QTextDocument> doc, QUrl url, QObject *pluginObj)
	: PluginObject_ { pluginObj }
	, Url_ { std::move (url) }
	{
		SetDocument (std::move (doc), {});
	}

	QObject* Document::GetBackendPlugin () const
	{
		return PluginObject_;
	}

	QObject* Document::GetQObject ()
	{
		return this;
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
