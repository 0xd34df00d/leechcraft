/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "corocontext.h"
#include <unordered_map>
#include <QThread>
#include <QtDebug>
#include "context.h"
#include "task.h"

namespace LC::Util
{
	ContextDeadException::ContextDeadException (const QString& name)
	: std::runtime_error { "coroutine's context " + name.toStdString () + " died" }
	{
	}

	CoroContext::CoroContext (const QString& name)
	: Name_ { name }
	{
	}

	CoroContext::CoroContext (const QObject& obj)
	: CoroContext { obj.objectName ().isEmpty () ? obj.metaObject ()->className () : obj.objectName () }
	{
	}

	CoroContext::~CoroContext ()
	{
		for (const auto [_, base, _] : Coros_)
			if (base->State_ == detail::PromiseBase::CoroState::Running)
				qFatal () << "destroying the context" << Name_ << "while its child coro is running; "
						<< "defer the destruction with `QObject::deleteLater()` or a timer";

		while (!Coros_.empty ())
		{
			const auto [handle, base, promise] = Coros_.front ();
			Coros_.pop_front ();

			std::erase_if (promise->Contexts_, [this] (const Registration& reg) { return reg.Ctx_ == this; });
			promise->DeadContexts_ << Name_;

			if (!handle.done ())
				handle.resume ();
		}
	}

	CoroContext& CoroContext::Of (QObject& obj)
	{
		if (obj.thread () != QThread::currentThread ())
		{
			using namespace std::string_literals;
			const auto& msg = "binding to a QObject "s + obj.metaObject ()->className () +
					" [" + obj.objectName ().toStdString () + "] from a different thread";
			throw std::runtime_error { msg };
		}

		thread_local std::unordered_map<QObject*, std::unique_ptr<CoroContext>> contexts;
		auto& ctx = contexts [&obj];
		if (!ctx)
		{
			ctx = std::make_unique<CoroContext> (obj);
			QObject::connect (&obj,
					&QObject::destroyed,
					[&obj]
					{
						if (!contexts.erase (&obj))
							qCritical () << "could not find" << &obj << "in the contexts map — threading messed up?";
					});
		}
		return *ctx;
	}
}
