/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/monocle/idocument.h"
#include "interfaces/monocle/ibackendplugin.h"

namespace LC
{
namespace Monocle
{
	class CoreLoadProxy : public QObject
	{
		Q_OBJECT

		const QString SourcePath_;
		IDocument_ptr Doc_;
		IRedirectProxy_ptr Proxy_;
	public:
		CoreLoadProxy (const IDocument_ptr&);
		CoreLoadProxy (const IRedirectProxy_ptr&);

		IDocument_ptr GetDocument () const;
	private slots:
		void handleRedirected (const QString& target);
		void handleSubproxy (const IDocument_ptr&, const QString&);
		void emitReady ();
	signals:
		void ready (IDocument_ptr, const QString&);
	};
}
}
