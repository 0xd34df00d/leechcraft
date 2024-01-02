/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/monocle/textdocumentadapter.h>

namespace LC::Monocle::Boop
{
	class Document : public TextDocumentAdapter
	{
		QObject * const PluginObject_;
		const QUrl Url_;
	public:
		explicit Document (QUrl, QObject*);

		QObject* GetBackendPlugin () const override;
		DocumentInfo GetDocumentInfo () const override;
		QUrl GetDocURL () const override;
	};

	IDocument_ptr LoadZip (const QString& file, QObject *pluginObj);
}
