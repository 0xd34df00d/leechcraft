/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fdopropsadaptor.h"
#include <cassert>
#include <QVariantMap>
#include <QStringList>
#include <QMetaMethod>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusVariant>
#include <QtDebug>

namespace LC::LMP::MPRIS
{
	FDOPropsAdaptor::FDOPropsAdaptor (QObject *parent)
	: QDBusAbstractAdaptor { parent }
	, Context_ { dynamic_cast<QDBusContext*> (parent) }
	{
		if (!Context_)
			qWarning () << Q_FUNC_INFO
					<< parent
					<< "doesn't implement QDBusContext";
		assert (Context_);
	}

	void FDOPropsAdaptor::Notify (const QString& iface, const QString& prop, const QVariant& val)
	{
		emit PropertiesChanged (iface, { { prop, val } }, {});
	}

	QDBusVariant FDOPropsAdaptor::Get (const QString& iface, const QString& propName)
	{
		QObject *child = 0;
		QMetaProperty prop;
		if (!GetProperty (iface, propName, &prop, &child))
			return QDBusVariant (QVariant ());

		return QDBusVariant (prop.read (child));
	}

	QVariantMap FDOPropsAdaptor::GetAll (const QString& iface)
	{
		QVariantMap result;

		const auto& adaptors = parent ()->findChildren<QDBusAbstractAdaptor*> ();
		for (const auto child : adaptors)
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

			for (int i = 0, cnt = mo->propertyCount (); i < cnt; ++i)
			{
				const auto& property = mo->property (i);
				result [property.name ()] = property.read (child);
			}
		}

		return result;
	}

	void FDOPropsAdaptor::Set (const QString& iface, const QString& propName, const QDBusVariant& value)
	{
		QObject *child = 0;
		QMetaProperty prop;
		if (!GetProperty (iface, propName, &prop, &child))
			return;

		if (!prop.isWritable ())
		{
			if (Context_ && Context_->calledFromDBus ())
				Context_->sendErrorReply (QDBusError::AccessDenied, propName + " isn't writable");
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

		if (Context_ && Context_->calledFromDBus ())
			Context_->sendErrorReply (QDBusError::InvalidMember, "no such property " + prop);

		return false;
	}
}
