/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <cstring>
#include <utility>
#include <QAbstractItemModelTester>
#include <QStringList>
#include <QtLogging>

namespace LC::Util
{
	/** @brief Like `QAbstractItemModelTester`, but keeping the failures instead of failing the current test function.
	 *
	 * The model is tested for as long as this object lives. The failures
	 * are neither logged nor reported to Qt Test, so a test may go on
	 * after one, or run the failing scenario again, as the shrinking of a
	 * random walk does.
	 *
	 * At most one such tester shall exist at a time.
	 */
	class CollectingModelTester
	{
		static inline CollectingModelTester *Current_ = nullptr;

		const QtMessageHandler NextHandler_;
		QStringList Failures_;
		QAbstractItemModelTester Tester_;
	public:
		explicit CollectingModelTester (QAbstractItemModel& model)
		: NextHandler_ { Install (*this) }
		, Tester_ { &model, QAbstractItemModelTester::FailureReportingMode::Warning }
		{
		}

		~CollectingModelTester ()
		{
			qInstallMessageHandler (NextHandler_);
			Current_ = nullptr;
		}

		/** @brief Returns the failures found so far, the earliest first.
		 */
		const QStringList& GetFailures () const
		{
			return Failures_;
		}
	private:
		static QtMessageHandler Install (CollectingModelTester& tester)
		{
			if (std::exchange (Current_, &tester))
				qFatal ("another CollectingModelTester exists already");
			return qInstallMessageHandler (&HandleMessage);
		}

		// QAbstractItemModelTester has no hook for its failures: all it can do besides
		// failing the test function or aborting is logging them as warnings of this category.
		static void HandleMessage (QtMsgType type, const QMessageLogContext& context, const QString& message)
		{
			if (type == QtWarningMsg && context.category && !std::strcmp (context.category, "qt.modeltest"))
				Current_->Failures_ << message;
			else
				Current_->NextHandler_ (type, context, message);
		}
	};
}
