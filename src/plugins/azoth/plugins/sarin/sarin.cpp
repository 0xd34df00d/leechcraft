/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sarin.h"
#include <QIcon>
#include <interfaces/azoth/iclentry.h>
#include <tox/tox.h>
#include "toxprotocol.h"

namespace LC::Azoth::Sarin
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		qRegisterMetaType<EntryStatus> ("EntryStatus");

		Proto_ = std::make_shared<ToxProtocol> (proxy, this);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Sarin";
	}

	void Plugin::Release ()
	{
		Proto_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Azoth Sarin";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Tox protocol support for Azoth.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QString Plugin::GetDiagInfoString () const
	{
		return QString { "Built with Tox %1.%2.%3, running with Tox %4.%5.%6" }
				.arg (TOX_VERSION_MAJOR)
				.arg (TOX_VERSION_MINOR)
				.arg (TOX_VERSION_PATCH)
				.arg (tox_version_major ())
				.arg (tox_version_minor ())
				.arg (tox_version_patch ());
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin" };
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return { Proto_.get () };
	}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_sarin, LC::Azoth::Sarin::Plugin);
