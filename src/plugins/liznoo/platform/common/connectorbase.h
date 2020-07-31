/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDBusConnection>
#include <QStringList>

namespace LC
{
namespace Liznoo
{
	class ConnectorBase : public QObject
	{
		Q_OBJECT
	protected:
		QDBusConnection SB_;
		const QString Service_;

		bool PowerEventsAvailable_ = false;

		ConnectorBase (const QString& service, const QByteArray& context, QObject *parent = nullptr);

		bool TryAutostart ();

		bool CheckSignals (const QString& path, const QStringList& signalsList);
	public:
		bool ArePowerEventsAvailable () const;
	signals:
		void gonnaSleep (int);
		void wokeUp ();
	};
}
}
