/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tasksadaptor.h"
#include <QCoreApplication>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusMetaType>
#include "core.h"

namespace LC
{
namespace DBusManager
{
	TasksAdaptor::TasksAdaptor (Tasks *parent)
	: QDBusAbstractAdaptor (parent)
	, Tasks_ (parent)
	{
		qDBusRegisterMetaType<QVariantList> ();
	}

	QStringList TasksAdaptor::GetHolders () const
	{
		return Tasks_->GetHolders ();
	}

	void TasksAdaptor::RowCount (const QString& name, const QDBusMessage& msg, int& result) const
	{
		HandleCall (Tasks_->RowCount (name), msg, result);
	}

	void TasksAdaptor::GetData (const QString& name, int r, int role, const QDBusMessage& msg, QVariantList& result) const
	{
		HandleCall (Tasks_->GetData (name, r, role), msg, result);
	}
}
}
