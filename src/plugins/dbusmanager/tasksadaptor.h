/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QStringList>

class QDBusMessage;

namespace LC
{
namespace DBusManager
{
	class Tasks;

	class TasksAdaptor : public QDBusAbstractAdaptor
	{
		Q_OBJECT

		Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.DBus.Tasks")

		Tasks *Tasks_;
	public:
		TasksAdaptor (Tasks*);
	public slots:
		QStringList GetHolders () const;
		void RowCount (const QString& holder, const QDBusMessage&, int&) const;
		void GetData (const QString& holder, int row, int role, const QDBusMessage&, QVariantList&) const;
	};
}
}
