/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "astrality.h"
#include <QIcon>
#include <QtDebug>
#include <Types>
#include <Debug>
#include <ConnectionManager>
#include <PendingStringList>
#include <util/sll/prelude.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iaccount.h>
#include "cmwrapper.h"
#include "accountwrapper.h"
#include "protowrapper.h"

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("azoth_astrality");
		Tp::registerTypes ();
		Tp::enableDebug (false);
		Tp::enableWarnings (false);
	}

	void Plugin::SecondInit ()
	{
		connect (Tp::ConnectionManager::listNames (),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleListNames (Tp::PendingOperation*)));
	}

	void Plugin::Release ()
	{
		for (const auto cmWrapper : Wrappers_)
			for (const auto protocol : cmWrapper->GetProtocols ())
				qobject_cast<ProtoWrapper*> (protocol)->Release ();

		qDeleteAll (Wrappers_);
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Astrality";
	}

	QString Plugin::GetName () const
	{
		return "Azoth Astrality";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for protocols provided by Telepathy.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return classes;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return Util::ConcatMap (Wrappers_, &CMWrapper::GetProtocols);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
	}

	void Plugin::handleListNames (Tp::PendingOperation *op)
	{
		auto psl = qobject_cast<Tp::PendingStringList*> (op);
		qDebug () << Q_FUNC_INFO << psl->result ();

		for (const auto& cmName : psl->result ())
		{
			auto cmw = new CMWrapper (cmName, Proxy_, this);
			Wrappers_ << cmw;

			connect (cmw,
					SIGNAL (gotProtoWrappers (QList<QObject*>)),
					this,
					SIGNAL (gotNewProtocols (QList<QObject*>)));
			connect (cmw,
					SIGNAL (gotProtoWrappers (QList<QObject*>)),
					this,
					SLOT (handleProtoWrappers (QList<QObject*>)));
		}
	}

	void Plugin::handleProtoWrappers (const QList<QObject*>& wrappers)
	{
		for (const auto obj : wrappers)
		{
			connect (obj,
					SIGNAL (gotEntity (LC::Entity)),
					this,
					SIGNAL (gotEntity (LC::Entity)));
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_astrality, LC::Azoth::Astrality::Plugin);
