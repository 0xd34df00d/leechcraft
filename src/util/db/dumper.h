/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QProcess>
#include <QFuture>
#include "dbconfig.h"

namespace LC
{
namespace Util
{
	class UTIL_DB_API Dumper : public QObject
	{
		Q_OBJECT

		QProcess * const Dumper_;
		QProcess * const Restorer_;

		bool HadError_ = false;

		int FinishedCount_ = 0;
	public:
		struct Finished {};
		struct Error
		{
			QString What_;

			Error (const QString& str)
			: What_ { str }
			{
			}
		};
		using Result_t = std::variant<Finished, Error>;
	private:
		QFutureInterface<Result_t> Iface_;
	public:
		Dumper (const QString& from, const QString& to, QObject* = nullptr);

		QFuture<Result_t> GetFuture ();
	private:
		void HandleProcessFinished (QProcess*);
		void HandleProcessError (const QProcess*);
		void ReportResult (const Result_t&);
	};
}
}
