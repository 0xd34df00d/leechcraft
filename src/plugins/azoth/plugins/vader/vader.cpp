/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vader.h"
#include <QIcon>
#include <QAction>
#include <QUrl>
#include <util/util.h>
#include <util/xpc/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include "mrimprotocol.h"
#include "mrimbuddy.h"
#include "vaderutil.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
        Util::InstallTranslator ("azoth_vader");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothvadersettings.xml");

		CoreProxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
		Proto_ = std::make_shared<MRIMProtocol> (AzothProxy_, CoreProxy_);
		emit gotNewProtocols ({ Proto_.get () });
	}

	void Plugin::Release ()
	{
		Proto_.reset ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Vader";
	}

	QString Plugin::GetName () const
	{
		return "Azoth Vader";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for the Mail.ru Agent protocol.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/plugins/azoth/plugins/vader/resources/images/vader.svg");
		return icon;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
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
		if (Proto_)
			return { Proto_.get () };
		else
			return {};
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::hookEntryActionAreasRequested (LC::IHookProxy_ptr,
			QObject*, QObject*)
	{
	}

	void Plugin::hookEntryActionsRequested (LC::IHookProxy_ptr proxy,
			QObject *entry)
	{
		if (!qobject_cast<MRIMBuddy*> (entry))
			return;

		if (!EntryServices_.contains (entry))
		{
			auto list = VaderUtil::GetBuddyServices (this,
					SLOT (entryServiceRequested ()));
			for (const auto act : list)
				act->setProperty ("Azoth/Vader/Entry", QVariant::fromValue<QObject*> (entry));
			EntryServices_ [entry] = list;
		}

		auto list = proxy->GetReturnValue ().toList ();
		for (const auto act : EntryServices_ [entry])
			list += QVariant::fromValue<QObject*> (act);
		proxy->SetReturnValue (list);
	}

	void Plugin::entryServiceRequested ()
	{
		const auto& url = sender ()->property ("URL").toString ();
		const auto buddyObj = sender ()->property ("Azoth/Vader/Entry").value<QObject*> ();
		const auto buddy = qobject_cast<MRIMBuddy*> (buddyObj);
		const auto& subst = VaderUtil::SubstituteNameDomain (url,
				buddy->GetHumanReadableID ());
		const auto& e = Util::MakeEntity (QUrl (subst),
				{},
				OnlyHandle | FromUserInitiated);

		CoreProxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_vader, LC::Azoth::Vader::Plugin);
