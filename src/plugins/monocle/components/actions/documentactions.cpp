/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "documentactions.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "components/navigation/navigator.h"
#include "components/services/recentlyopenedmanager.h"
#include "interfaces/monocle/iknowfileextensions.h"
#include "xmlsettingsmanager.h"

namespace LC::Monocle
{
	DocumentActions::DocumentActions (QToolBar& bar, const Deps& deps)
	: Deps_ { deps }
	{
		bar.addWidget (MakeOpenButton ());
	}

	void DocumentActions::HandleDocument (const IDocument& doc)
	{
	}

	QWidget* DocumentActions::MakeOpenButton ()
	{
		auto open = new QAction (tr ("Open..."), this);
		open->setProperty ("ActionIcon", "document-open");
		open->setShortcut (QString ("Ctrl+O"));
		connect (open,
				&QAction::triggered,
				this,
				&DocumentActions::RunOpenDialog);

		auto roMenu = Deps_.RecentlyOpenedManager_.CreateOpenMenu (&Deps_.DocTabWidget_,
				[this] (const QString& path)
				{
					const QFileInfo fi { path };
					if (!fi.exists ())
						QMessageBox::critical (&Deps_.DocTabWidget_,
								"Monocle"_qs,
								tr ("Seems like file %1 doesn't exist anymore.")
										.arg ("<em>" + fi.fileName () + "</em>"));
					else
						Deps_.Navigator_.OpenDocument (path);
				});

		auto openButton = new QToolButton;
		openButton->setDefaultAction (open);
		openButton->setMenu (roMenu);
		openButton->setPopupMode (QToolButton::MenuButtonPopup);
		return openButton;
	}

	void DocumentActions::RunOpenDialog ()
	{
		const auto& extPlugins = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IKnowFileExtensions*> ();
		QStringList filters;
		QList<QString> allExts;
		for (const auto plugin : extPlugins)
			for (const auto& info : plugin->GetKnownFileExtensions ())
			{
				const auto& mapped = Util::Map (info.Extensions_,
						[] (const QString& str) { return "*." + str; });
				allExts += mapped;
				filters << info.Description_ + " (" + QStringList { mapped }.join (' ') + ")";
			}
		if (!allExts.isEmpty ())
			filters.prepend (tr ("Known files") + " (" + QStringList { allExts }.join (' ') + ")");
		filters << tr ("All files") + " (*.*)";

		constexpr auto LastOpen = "LastOpenFileName";
		const auto& prevPath = XmlSettingsManager::Instance ().Property (LastOpen, QDir::homePath ()).toString ();
		const auto& path = QFileDialog::getOpenFileName (&Deps_.DocTabWidget_,
				tr ("Select file"),
				prevPath,
				filters.join (";;"_qs));
		if (path.isEmpty ())
			return;

		XmlSettingsManager::Instance ().setProperty (LastOpen, QFileInfo (path).absolutePath ());
		Deps_.Navigator_.OpenDocument (path);
	}
}
