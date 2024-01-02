/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <util/monocle/textdocumentadapter.h>

namespace LC
{
namespace Monocle
{
namespace FXB
{
	class Document : public TextDocumentAdapter
	{
		DocumentInfo Info_;
		QUrl DocURL_;

		QObject *Plugin_;
	public:
		Document (const QString&, QObject*);

		QObject* GetBackendPlugin () const;
		QObject* GetQObject ();
		DocumentInfo GetDocumentInfo () const;
		QUrl GetDocURL () const;

		void RequestNavigation (int);
	};
}
}
}
