/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "snails.h"
#include <QIcon>
#include <util/util.h>
#include <util/xsd/wkfontswidget.h>
#include <util/shortcuts/shortcutmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "mailtab.h"
#include "xmlsettingsmanager.h"
#include "accountslistwidget.h"
#include "core.h"
#include "progressmanager.h"
#include "composemessagetab.h"
#include "accountsmanager.h"
#include "composemessagetabfactory.h"
#include "msgtemplatesmanager.h"
#include "templateseditorwidget.h"
#include "storage.h"

namespace LC
{
namespace Snails
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("snails");

		Proxy_ = proxy;

		MailTabClass_ =
		{
			"mail",
			tr ("Mail"),
			tr ("Mail tab."),
			GetIcon (),
			65,
			TFOpenableByRequest
		};
		ComposeTabClass_ =
		{
			"compose",
			tr ("Compose mail"),
			tr ("Allows one to compose outgoing mail messages."),
			proxy->GetIconThemeManager ()->GetIcon ("mail-message-new"),
			60,
			TFOpenableByRequest
		};

		ComposeMessageTab::SetParentPlugin (this);
		ComposeMessageTab::SetTabClassInfo (ComposeTabClass_);

		Core::Instance ().SetProxy (proxy);

		Storage_ = std::make_shared<Storage> ();

		ProgressMgr_ = new ProgressManager;

		ShortcutsMgr_ = new Util::ShortcutManager { proxy, this };
		ShortcutsMgr_->SetObject (this);

		MailTab::FillShortcutsManager (ShortcutsMgr_, proxy);

		AccsMgr_ = std::make_shared<AccountsManager> (ProgressMgr_, Storage_.get ());
		TemplatesMgr_ = new MsgTemplatesManager;
		ComposeTabFactory_ = new ComposeMessageTabFactory { AccsMgr_.get (), TemplatesMgr_ };

		connect (ComposeTabFactory_,
				SIGNAL (gotTab (QString, QWidget*)),
				this,
				SLOT (handleNewTab (QString, QWidget*)));

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "snailssettings.xml");

		XSD_->SetCustomWidget ("AccountsWidget", new AccountsListWidget { AccsMgr_.get () });

		WkFontsWidget_ = new Util::WkFontsWidget { &XmlSettingsManager::Instance () };
		XSD_->SetCustomWidget ("FontsSelector", WkFontsWidget_);
	}

	void Plugin::SecondInit ()
	{
		XSD_->SetCustomWidget ("TemplatesWidget", new TemplatesEditorWidget { TemplatesMgr_ });

		AccsMgr_->InitWithPlugins ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Snails";
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();

		AccsMgr_.reset ();
		Storage_.reset ();
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
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return
		{
			MailTabClass_,
			ComposeTabClass_
		};
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "mail")
		{
			const auto mt = new MailTab { Proxy_, AccsMgr_.get (), ComposeTabFactory_,
					Storage_.get (), MailTabClass_, ShortcutsMgr_, this };
			handleNewTab (MailTabClass_.VisibleName_, mt);
		}
		else if (tabClass == "compose")
		{
			const auto ct = ComposeTabFactory_->MakeTab ();
			handleNewTab (ct->GetTabClassInfo ().VisibleName_, ct);
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
		return ProgressMgr_->GetRepresentation ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& sequences)
	{
		ShortcutsMgr_->SetShortcut (id, sequences);
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return ShortcutsMgr_->GetActionInfo ();
	}

	void Plugin::handleNewTab (const QString& name, QWidget *mt)
	{
		if (const auto iwfs = qobject_cast<IWkFontsSettable*> (mt))
			WkFontsWidget_->RegisterSettable (iwfs);

		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (name, mt);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_snails, LC::Snails::Plugin);

