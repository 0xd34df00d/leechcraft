/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDBusAbstractAdaptor>
#include <QStringList>

class QDBusMessage;

namespace LC
{
namespace DBusManager
{
	class General;

	class GeneralAdaptor : public QDBusAbstractAdaptor
	{
		Q_OBJECT

		Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.DBus.General")
		Q_PROPERTY (QString OrganizationName READ GetOrganizationName)
		Q_PROPERTY (QString ApplicationName READ GetApplicationName)

		General *General_;
	public:
		GeneralAdaptor (General*);

		QString GetOrganizationName () const;
		QString GetApplicationName () const;
	public slots:
		QStringList GetLoadedPlugins ();
		void GetDescription (const QString& name, const QDBusMessage&, QString&);
		void GetIcon (const QString& name, int dimension, const QDBusMessage&, QByteArray&);
	};
}
}
