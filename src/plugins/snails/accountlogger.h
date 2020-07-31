/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <atomic>
#include <QObject>

class QFile;
class QDateTime;

namespace LC
{
namespace Snails
{
	class AccountLogger : public QObject
	{
		Q_OBJECT

		std::atomic_bool Enabled_ { false };
		const QString AccName_;
		std::shared_ptr<QFile> File_;
	public:
		AccountLogger (const QString&, QObject* = nullptr);

		void SetEnabled (bool);

		void Log (const QString& context, int connId, const QString& msg);
	private slots:
		void writeLog (const QString&);
	signals:
		void gotLog (const QDateTime& time, const QString& context,
				int connId, const QString& msg);
	};
}
}
