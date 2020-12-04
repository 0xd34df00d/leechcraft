/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <variant>
#include <QObject>
#include "dbconfig.h"

template<typename>
class QFuture;

template<typename>
class QFutureInterface;

namespace LC
{
namespace Util
{
	class UTIL_DB_API ConsistencyChecker : public QObject
										 , public std::enable_shared_from_this<ConsistencyChecker>
	{
		const QString DBPath_;
		const QString DialogContext_;

		friend class FailedImpl;

		ConsistencyChecker (const QString& dbPath, const QString& dialogContext, QObject* = nullptr);
	public:
		static std::shared_ptr<ConsistencyChecker> Create (const QString& dbPath, const QString& dialogContext);

		struct DumpFinished
		{
			qint64 OldFileSize_;
			qint64 NewFileSize_;
		};
		struct DumpError
		{
			QString Error_;
		};
		using DumpResult_t = std::variant<DumpFinished, DumpError>;

		struct Succeeded {};
		struct IFailed
		{
			virtual QFuture<DumpResult_t> DumpReinit () = 0;

			// Not having a virtual dtor here is fine, since its subclasses will
			// only be deleted through a shared_ptr, which remembers the exact
			// type of the constructed object.
		};
		using Failed = std::shared_ptr<IFailed>;

		using CheckResult_t = std::variant<Succeeded, Failed>;

		QFuture<CheckResult_t> StartCheck ();
	private:
		CheckResult_t CheckDB ();

		QFuture<DumpResult_t> DumpReinit ();
		void DumpReinitImpl (QFutureInterface<DumpResult_t>);

		void HandleDumperFinished (QFutureInterface<DumpResult_t>, const QString&);
	};
}
}
