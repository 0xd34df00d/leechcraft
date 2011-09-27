/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "autoidler.h"
#include <QIcon>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/iproxyobject.h>
#include <interfaces/iaccount.h>
#include "xmlsettingsmanager.h"
#include "3dparty/idle.h"

namespace LeechCraft
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
		return QIcon (":/plugins/azoth/plugins/autoidler/resources/images/autoidler.svg");
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
		if (seconds && seconds % 60)
			return;

		if (!XmlSettingsManager::Instance ().property ("EnableAutoidler").toBool ())
			return;

		const int mins = seconds / 60;
		if (!mins &&
				!OldStatuses_.isEmpty ())
		{
			Q_FOREACH (QObject *accObj, AzothProxy_->GetAllAccounts ())
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

		Q_FOREACH (QObject *accObj, AzothProxy_->GetAllAccounts ())
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);

			const EntryStatus& oldStatus = acc->GetState ();
			if (oldStatus.State_ == SOffline)
				continue;

			if (!OldStatuses_.contains (accObj))
				OldStatuses_ [accObj] = oldStatus;

			acc->ChangeState (status);
		}
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_autoidler, LeechCraft::Azoth::Autoidler::Plugin);
