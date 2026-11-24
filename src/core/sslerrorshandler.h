/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <util/threads/coro/taskfwd.h>
#include <util/threads/coro/corocontext.h>

class QNetworkReply;
class QSslError;

namespace LC
{
	class SslErrorsHandler : public QObject
	{
		enum class Decision
		{
			Undecided,
			Accepted,
			Rejected
		};

		QHash<QString, Decision> SessionDecisions_;
		QSet<QString> OpenQuestions_;
	public:
		using QObject::QObject;

		void Handle (QNetworkReply*, const QList<QSslError>&);
	private:
		Util::ContextTask<void> Ask (QString host, QList<QSslError>);

		LC_CORO_CONTEXT
	};
}
