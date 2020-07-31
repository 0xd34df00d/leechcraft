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

class QDBusContext;

namespace LC
{
namespace LMP
{
namespace MPRIS
{
	class FDOPropsAdaptor : public QDBusAbstractAdaptor
	{
		Q_OBJECT

		Q_CLASSINFO ("D-Bus Interface", "org.freedesktop.DBus.Properties")

		QDBusContext * const Context_;
	public:
		FDOPropsAdaptor (QObject*);

		void Notify (const QString& iface, const QString& prop, const QVariant& val);
	public slots:
		QDBusVariant Get (const QString& iface, const QString& prop);
		QVariantMap GetAll (const QString& iface);
		void Set (const QString& iface, const QString& prop, const QDBusVariant&);
	private:
		bool GetProperty (const QString&, const QString&, QMetaProperty*, QObject**) const;
	signals:
		void PropertiesChanged (const QString&, const QVariantMap&, const QStringList&);
	};
}
}
}
