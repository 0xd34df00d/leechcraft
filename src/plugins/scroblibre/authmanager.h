/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>
#include "submitinfo.h"

class QTimer;

namespace LC
{
struct Entity;

namespace Scroblibre
{
	class SingleAccAuth;

	class AuthManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		QHash<QUrl, QHash<QString, SingleAccAuth*>> AccAuths_;

		SubmitInfo LastSubmit_;

		QTimer * const SubmitTimer_;
	public:
		AuthManager (ICoreProxy_ptr, QObject* = 0);

		void HandleAudio (const Media::AudioInfo&);
		void HandleStopped ();
	public slots:
		void handleAccountAdded (const QUrl& url, const QString& login);
		void handleAccountRemoved (const QUrl& url, const QString& login);
	private slots:
		void submit ();
	};
}
}
