/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toolbaractions.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "components/actions/documentactions.h"
#include "components/navigation/navigator.h"
#include "components/services/recentlyopenedmanager.h"
#include "interfaces/monocle/iknowfileextensions.h"
#include "xmlsettingsmanager.h"

namespace LC::Monocle
{
	ToolbarActions::ToolbarActions (QToolBar& bar, const Deps& deps)
	: Deps_ { deps }
	, Bar_ { bar }
	{
		Bar_.addWidget (MakeOpenButton ());
	}

	ToolbarActions::~ToolbarActions () = default;

	void ToolbarActions::HandleDoc (IDocument& doc)
	{
		DocActions_ = std::make_unique<DocumentActions> (doc, DocumentActions::Deps {
				.DocTabWidget_ = Deps_.DocTabWidget_,
			});
		for (const auto& entry : DocActions_->GetEntries ())
			Util::Visit (entry,
					[this] (QAction *action) { Bar_.addAction (action); },
					[this] (QWidget *widget) { Bar_.addWidget (widget); },
					[this] (DocumentActions::Separator) { Bar_.addSeparator ()->setParent (DocActions_.get ()); });
	}

	QWidget* ToolbarActions::MakeOpenButton ()
	{
		auto open = new QAction (tr ("Open..."), this);
		open->setProperty ("ActionIcon", "document-open");
		open->setShortcut (QString ("Ctrl+O"));
		connect (open,
				&QAction::triggered,
				this,
				&ToolbarActions::RunOpenDialog);

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

	void ToolbarActions::RunOpenDialog ()
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
