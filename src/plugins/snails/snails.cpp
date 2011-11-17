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

#include "snails.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "mailtab.h"
#include "xmlsettingsmanager.h"
#include "accountslistwidget.h"
#include "core.h"
#include "progressmanager.h"
#include "composemessagetab.h"

namespace LeechCraft
{
namespace Snails
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		MailTabClass_ = { "mail", tr ("Mail"),
				tr ("Mail tab."),
				GetIcon (), 55, TFOpenableByRequest };
		ComposeTabClass_ = { "compose", tr ("Compose mail"),
				tr ("Allows one to compose outgoing mail messages."),
				QIcon (), 60, TFOpenableByRequest };

		ComposeMessageTab::SetParentPlugin (this);
		ComposeMessageTab::SetTabClassInfo (ComposeTabClass_);

		Core::Instance ().SetProxy (proxy);

		connect (&Core::Instance (),
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (LeechCraft::Entity,int*,QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity,int*,QObject**)));

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "snailssettings.xml");

		XSD_->SetCustomWidget ("AccountsWidget", new AccountsListWidget ());
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Snails";
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();
	}

	QString Plugin::GetName () const
	{
		return "Snails";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("LeechCraft mail client.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/resources/images/snails.svg");
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		TabClasses_t result;
		result << MailTabClass_;
		result << ComposeTabClass_;
		return result;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "mail")
		{
			auto mt = new MailTab (MailTabClass_, this);

			connect (mt,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));

			emit addNewTab (MailTabClass_.VisibleName_, mt);
			emit raiseTab (mt);
		}
		else if (tabClass == "compose")
		{
			auto ct = new ComposeMessageTab ();

			connect (ct,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));

			emit addNewTab (ct->GetTabClassInfo ().VisibleName_, ct);
			emit raiseTab (ct);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return Core::Instance ().GetProgressManager ()->GetRepresentation ();
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_snails, LeechCraft::Snails::Plugin);

