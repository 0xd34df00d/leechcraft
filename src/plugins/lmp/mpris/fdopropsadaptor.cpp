/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "fdopropsadaptor.h"
#include <QVariantMap>
#include <QStringList>
#include <QMetaMethod>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusVariant>
#include <QtDebug>

namespace LeechCraft
{
namespace LMP
{
namespace MPRIS
{
	FDOPropsAdaptor::FDOPropsAdaptor (QObject *parent)
	: QDBusAbstractAdaptor (parent)
	{
	}

	void FDOPropsAdaptor::Notify (const QString& iface, const QString& prop, const QVariant& val)
	{
		QVariantMap changes;
		changes [prop] = val;
		emit PropertiesChanged (iface, changes, QStringList ());
	}

	QDBusVariant FDOPropsAdaptor::Get (const QString& iface, const QString& propName)
	{
		QObject *child = 0;
		QMetaProperty prop;
		if (!GetProperty (iface, propName, &prop, &child))
			return QDBusVariant (QVariant ());

		return QDBusVariant (prop.read (child));
	}

	void FDOPropsAdaptor::Set (const QString& iface, const QString& propName, const QDBusVariant& value)
	{
		QObject *child = 0;
		QMetaProperty prop;
		if (!GetProperty (iface, propName, &prop, &child))
			return;

		if (!prop.isWritable ())
		{
			auto context = dynamic_cast<QDBusContext*> (parent ());
			if (context->calledFromDBus ())
				context->sendErrorReply (QDBusError::AccessDenied, propName + " isn't writable");
			return;
		}

		prop.write (child, value.variant ());
	}

	bool FDOPropsAdaptor::GetProperty (const QString& iface, const QString& prop, QMetaProperty *propObj, QObject **childObject) const
	{
		auto adaptors = parent ()->findChildren<QDBusAbstractAdaptor*> ();
		for (const auto& child : adaptors)
		{
			const auto mo = child->metaObject ();

			if (!iface.isEmpty ())
			{
				const auto idx = mo->indexOfClassInfo ("D-Bus Interface");
				if (idx == -1)
					continue;

				const auto& info = mo->classInfo (idx);
				if (iface != info.value ())
					continue;
			}

			const auto idx = mo->indexOfProperty (prop.toUtf8 ().constData ());
			if (idx != -1)
			{
				*propObj = mo->property (idx);
				*childObject = child;
				return true;
			}
		}

		auto context = dynamic_cast<QDBusContext*> (parent ());
		if (context->calledFromDBus ())
			context->sendErrorReply (QDBusError::InvalidMember, "no such property " + prop);

		return false;
	}
}
}
}
