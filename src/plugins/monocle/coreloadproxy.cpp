/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coreloadproxy.h"
#include <QFile>
#include <QTimer>
#include <QUrl>
#include <QtDebug>
#include "interfaces/monocle/iredirectproxy.h"
#include "core.h"


namespace LC::Monocle
{
	CoreLoadProxy::CoreLoadProxy (const IDocument_ptr& doc)
	: SourcePath_ { doc->GetDocURL ().toLocalFile () }
	, Doc_ { doc }
	{
		QTimer::singleShot (0,
				this,
				&CoreLoadProxy::EmitReady);
	}

	CoreLoadProxy::CoreLoadProxy (const IRedirectProxy_ptr& proxy)
	: SourcePath_ { proxy->GetRedirectSource () }
	, Proxy_ { proxy }
	{
		connect (proxy->GetQObject (),
				SIGNAL (ready (QString)),
				this,
				SLOT (handleRedirected (QString)));
	}

	void CoreLoadProxy::EmitReady ()
	{
		emit ready (Doc_, SourcePath_);
		deleteLater ();
	}

	void CoreLoadProxy::handleRedirected (const QString& target)
	{
		auto subProxy = Core::Instance ().LoadDocument (target);
		if (!subProxy)
		{
			EmitReady ();
			return;
		}

		connect (subProxy,
				&CoreLoadProxy::ready,
				this,
				[this] (const IDocument_ptr& doc, const QString& path)
				{
					qDebug () << Q_FUNC_INFO;
					if (!doc)
						qWarning () << "redirection failed from"
								<< SourcePath_
								<< "to"
								<< path;

					Doc_ = doc;

					if (doc)
						QObject::connect (doc->GetQObject (),
								&QObject::destroyed,
								[path]
								{
									qDebug () << "removing" << path;
									QFile::remove (path);
								});

					EmitReady ();
				});
	}
}
