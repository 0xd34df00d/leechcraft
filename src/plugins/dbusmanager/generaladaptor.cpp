/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "generaladaptor.h"
#include <QCoreApplication>
#include <QDBusMessage>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include "core.h"
#include "common.h"

namespace LC
{
namespace DBusManager
{
	GeneralAdaptor::GeneralAdaptor (General *parent)
	: QDBusAbstractAdaptor (parent)
	, General_ (parent)
	{
	}

	QString GeneralAdaptor::GetOrganizationName () const
	{
		return QCoreApplication::organizationName ();
	}

	QString GeneralAdaptor::GetApplicationName () const
	{
		return QCoreApplication::applicationName ();
	}

	QStringList GeneralAdaptor::GetLoadedPlugins ()
	{
		return General_->GetLoadedPlugins ();
	}

	void GeneralAdaptor::GetDescription (const QString& name, const QDBusMessage& msg, QString& result)
	{
		HandleCall (General_->GetDescription (name), msg, result);
	}

	void GeneralAdaptor::GetIcon (const QString& name, int dim, const QDBusMessage& msg, QByteArray& result)
	{
		HandleCall (General_->GetIcon (name, dim), msg, result);
	}
}
}
