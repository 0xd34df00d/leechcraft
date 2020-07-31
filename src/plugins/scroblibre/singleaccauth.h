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
#include <interfaces/core/icoreproxy.h>
#include "submitinfo.h"

namespace LC
{
namespace Scroblibre
{
	class SingleAccAuth : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		const QUrl BaseURL_;
		const QString Login_;

		QString SID_;
		QUrl NowPlayingUrl_;
		QUrl SubmissionsUrl_;

		bool ReauthScheduled_ = false;

		QList<SubmitInfo> Queue_;
		SubmitInfo LastSubmit_;
	public:
		SingleAccAuth (const QUrl& url, const QString& login, ICoreProxy_ptr, QObject*);

		void SetNP (const SubmitInfo&);
		void Submit (const SubmitInfo&);
	private:
		void LoadQueue ();
		void SaveQueue () const;
	private slots:
		void reauth (bool failed = false);

		void rotateSubmitQueue ();

		void handleHSFinished ();
		void handleSubmission ();
	};
}
}
