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
#include <QVariantMap>
#include <QDateTime>
#include <interfaces/core/icoreproxy.h>

class QNetworkReply;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkConnection;

	class LongPollManager : public QObject
	{
		Q_OBJECT

		VkConnection * const Conn_;
		const ICoreProxy_ptr Proxy_;

		QString LPKey_;
		QString LPServer_;
		qulonglong LPTS_ = 0;

		QUrl LPURLTemplate_;

		int PollErrorCount_ = 0;

		bool ShouldStop_ = false;

		int WaitTimeout_ = 25;

		QDateTime LastPollDT_;

		QNetworkReply *CurrentPollReply_ = nullptr;
	public:
		LongPollManager (VkConnection*, ICoreProxy_ptr);

		void ForceServerRequery ();
		void Stop ();
	private:
		QUrl GetURLTemplate () const;
		void HandlePollError (QNetworkReply*);
	public slots:
		void start ();
	private slots:
		void poll ();

		void handlePollFinished ();
		void handleGotLPServer ();
	signals:
		void listening ();
		void stopped ();
		void pollError ();
		void gotPollData (const QVariantMap&);
	};
}
}
}
