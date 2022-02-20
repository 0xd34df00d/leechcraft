/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "blogique.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "accountslistwidget.h"
#include "blogiquewidget.h"
#include "commentswidget.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		qRegisterMetaType<LC::Blogique::CommentsWidget::CommentIDs_t> ("LC::Blogique::CommentIDs_t");
		qRegisterMetaTypeStreamOperators<LC::Blogique::CommentsWidget::CommentIDs_t> ();

		Util::InstallTranslator ("blogique");
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog);
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"blogiquesettings.xml");
		XmlSettingsDialog_->SetCustomWidget ("AccountsWidget", new AccountsListWidget);

		Core::Instance ().SetCoreProxy (proxy);

		BlogiqueWidget::SetParentMultiTabs (this);

		TabClassInfo tabClass =
		{
			"Blogique",
			"Blogique",
			GetInfo (),
			GetIcon (),
			50,
			TFOpenableByRequest | TFSuggestOpening
		};
		TabClasses_ << tabClass;

		connect (&Core::Instance (),
				SIGNAL (gotEntity (LC::Entity)),
				this,
				SIGNAL (gotEntity (LC::Entity)));

		ExportAction_ = new QAction (proxy->GetIconThemeManager ()->GetIcon ("document-export"),
				tr ("Export blog"), this);
		connect (ExportAction_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (exportBlog ()));
	}

	void Plugin::SecondInit ()
	{
		Core::Instance ().DelayedProfilesUpdate ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blogique";
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();
	}

	QString Plugin::GetName () const
	{
		return "Blogique";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Blogging client");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "Blogique")
			CreateTab ();
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Blogique.Plugins.IBlogPlatformPlugin";
		return classes;
	}

	void Plugin::AddPlugin (QObject* plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace area) const
	{
		QList<QAction*> result;

		switch (area)
		{
			case ActionsEmbedPlace::ToolsMenu:
				result << ExportAction_;
				break;
			default:
				break;
		}

		return result;
	}

	void Plugin::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		for (const auto& recInfo : infos)
		{
			QDataStream str (recInfo.Data_);
			qint8 version;
			str >> version;

			if (version == 1)
			{
				auto tab = Core::Instance ().CreateBlogiqueWidget ();
				Entry e;
				str >> e.Subject_
						>> e.Content_
						>> e.Date_
						>> e.Tags_
						>> e.Target_
						>> e.PostOptions_
						>> e.CustomData_;
				QByteArray accId;
				str >> accId;
				tab->FillWidget (e, accId);

				GetProxyHolder ()->GetRootWindowsManager ()->AddTab (e.Subject_, tab);
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown version"
						<< version;
		}
	}

	bool Plugin::HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const
	{
		return false;
	}

	void Plugin::CreateTab ()
	{
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (GetName (), Core::Instance ().CreateBlogiqueWidget ());
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_blogique, LC::Blogique::Plugin);
