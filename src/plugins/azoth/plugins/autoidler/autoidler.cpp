/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "autoidler.h"
#include <QIcon>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include "xmlsettingsmanager.h"
#include "3dparty/idle.h"

namespace LC
{
namespace Azoth
{
namespace Autoidler
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		IdleSeconds_ = 0;

		Translator_.reset (Util::InstallTranslator ("azoth_autoidler"));

		Proxy_ = proxy;

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog);
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothautoidlersettings.xml");

		Idle_.reset (new Idle);
		connect (Idle_.get (),
				SIGNAL (secondsIdle (int)),
				this,
				SLOT (handleIdle (int)));
	}

	void Plugin::SecondInit ()
	{
		Idle_->start ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Autoidler";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Autoidler";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Automatically updates statuses depending on idle time.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	int Plugin::GetInactiveSeconds ()
	{
		return IdleSeconds_;
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::handleIdle (int seconds)
	{
		IdleSeconds_ = seconds;
		if (seconds && (seconds % 60) >= Idle_->interval () / 1000)
			return;

		if (!XmlSettingsManager::Instance ().property ("EnableAutoidler").toBool ())
			return;

		const int mins = seconds / 60;
		if (!mins &&
				!OldStatuses_.isEmpty ())
		{
			for (QObject *accObj : AzothProxy_->GetAllAccounts ())
			{
				if (!OldStatuses_.contains (accObj))
					continue;

				IAccount *acc = qobject_cast<IAccount*> (accObj);
				acc->ChangeState (OldStatuses_ [accObj]);
			}

			OldStatuses_.clear ();
		}

		if (!mins)
			return;

		EntryStatus status;
		if (mins == XmlSettingsManager::Instance ().property ("AwayTimeout").toInt ())
			status = EntryStatus (SAway,
					XmlSettingsManager::Instance ().property ("AwayText").toString ());
		else if (mins == XmlSettingsManager::Instance ().property ("NATimeout").toInt ())
			status = EntryStatus (SXA,
					XmlSettingsManager::Instance ().property ("NAText").toString ());
		else
			return;

		for (QObject *accObj : AzothProxy_->GetAllAccounts ())
		{
			auto acc = qobject_cast<IAccount*> (accObj);

			const auto& accState = acc->GetState ();
			if (!OldStatuses_.contains (accObj) &&
					accState.State_ != State::SOnline &&
					accState.State_ != State::SChat)
				continue;

			if (!OldStatuses_.contains (accObj))
				OldStatuses_ [accObj] = accState;
			acc->ChangeState (status);
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_autoidler, LC::Azoth::Autoidler::Plugin);
