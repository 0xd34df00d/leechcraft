/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coreloadproxy.h"
#include <QTimer>
#include <QUrl>
#include <QtDebug>
#include "interfaces/monocle/iredirectproxy.h"
#include "core.h"
#include "converteddoccleaner.h"

namespace LC
{
namespace Monocle
{
	CoreLoadProxy::CoreLoadProxy (const IDocument_ptr& doc)
	: SourcePath_ { doc->GetDocURL ().toLocalFile () }
	, Doc_ { doc }
	{
		QTimer::singleShot (0,
				this,
				SLOT (emitReady ()));
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

	IDocument_ptr CoreLoadProxy::GetDocument () const
	{
		return Doc_;
	}

	void CoreLoadProxy::handleRedirected (const QString& target)
	{
		auto subProxy = Core::Instance ().LoadDocument (target);
		if (!subProxy)
		{
			emitReady ();
			return;
		}

		connect (subProxy,
				SIGNAL (ready (IDocument_ptr, QString)),
				this,
				SLOT (handleSubproxy (IDocument_ptr, QString)));
	}

	void CoreLoadProxy::handleSubproxy (const IDocument_ptr& doc, const QString& path)
	{
		qDebug () << Q_FUNC_INFO;
		if (!doc)
			qWarning () << Q_FUNC_INFO
					<< "redirection failed from"
					<< SourcePath_
					<< "to"
					<< path;

		Doc_ = doc;
		new ConvertedDocCleaner { Doc_ };

		emitReady ();
	}

	void CoreLoadProxy::emitReady ()
	{
		emit ready (Doc_, SourcePath_);
		deleteLater ();
	}
}
}
