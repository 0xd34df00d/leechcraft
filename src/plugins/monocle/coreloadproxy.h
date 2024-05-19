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


namespace LC::Monocle
{
	class CoreLoadProxy : public QObject
	{
		Q_OBJECT

		const QString SourcePath_;
		IDocument_ptr Doc_;
		IRedirectProxy_ptr Proxy_;
	public:
		explicit CoreLoadProxy (const IDocument_ptr&);
		explicit CoreLoadProxy (const IRedirectProxy_ptr&);
	private:
		void EmitReady ();
	private slots:
		void handleRedirected (const QString& target);
	signals:
		void ready (IDocument_ptr, const QString&);
	};
}
