/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fsbrowserwidget.h"
#include <QDir>
#include <QAction>
#include <util/util.h>
#include "fsmodel.h"
#include "util.h"
#include "player.h"
#include "core.h"
#include "localcollection.h"
#include "audiopropswidget.h"
#include "xmlsettingsmanager.h"

namespace LC::LMP
{
	FSBrowserWidget::FSBrowserWidget (QWidget *parent)
	: QWidget { parent }
	, FSModel_ { new FSModel { this } }
	, DirCollection_ { new QAction { QString {}, this } }
	, ViewProps_ { new QAction { tr ("Show track properties"), this } }
	{
		Ui_.setupUi (this);

		FSModel_->setReadOnly (true);
		FSModel_->setRootPath (QDir::rootPath ());
		Ui_.FSTree_->setModel (FSModel_);

		auto addToPlaylist = new QAction (tr ("Add to playlist"), this);
		addToPlaylist->setProperty ("ActionIcon", "list-add");
		connect (addToPlaylist,
				&QAction::triggered,
				this,
				&FSBrowserWidget::LoadFromFSBrowser);
		Ui_.FSTree_->addAction (addToPlaylist);

		DirCollection_->setProperty ("WatchActionIconChange", true);
		Ui_.FSTree_->addAction (DirCollection_);

		Ui_.FSTree_->addAction (Util::CreateSeparator (this));

		ViewProps_->setProperty ("ActionIcon", "document-properties");
		connect (ViewProps_,
				&QAction::triggered,
				this,
				[this]
				{
					const auto& index = Ui_.FSTree_->currentIndex ();
					if (index.isValid ())
						AudioPropsWidget::MakeDialog ()->SetProps (FSModel_->fileInfo (index).absoluteFilePath ());
				});
		Ui_.FSTree_->addAction (ViewProps_);

		connect (Ui_.FSTree_->selectionModel (),
				&QItemSelectionModel::currentRowChanged,
				this,
				&FSBrowserWidget::UpdateActions);

		connect (Core::Instance ().GetLocalCollection (),
				&LocalCollection::rootPathsChanged,
				this,
				[this] { UpdateActions (Ui_.FSTree_->currentIndex ()); });
	}

	void FSBrowserWidget::showEvent (QShowEvent *event)
	{
		QWidget::showEvent (event);

		if (!ColumnsBeenResized_)
		{
			constexpr auto proportion = 0.6;
			Ui_.FSTree_->setColumnWidth (0, Ui_.FSTree_->width () * proportion);
			ColumnsBeenResized_ = true;
		}
	}

	void FSBrowserWidget::AssociatePlayer (Player *player)
	{
		Player_ = player;
	}

	void FSBrowserWidget::UpdateActions (const QModelIndex& index)
	{
		const auto& fi = FSModel_->fileInfo (index);
		ViewProps_->setEnabled (fi.isFile ());

		const auto& path = fi.absoluteFilePath ();

		if (DirCollectionConn_)
		{
			QObject::disconnect (DirCollectionConn_);
			DirCollectionConn_ = {};
		}

		switch (Core::Instance ().GetLocalCollection ()->GetDirStatus (path))
		{
		case LocalCollection::DirStatus::None:
			DirCollection_->setText (tr ("Add to collection..."));
			DirCollection_->setEnabled (true);
			DirCollectionConn_ = connect (DirCollection_,
					&QAction::triggered,
					this,
					[this]
					{
						const auto& index = Ui_.FSTree_->currentIndex ();
						const auto& path = FSModel_->fileInfo (index).absoluteFilePath ();
						Core::Instance ().GetLocalCollection ()->Scan (path);
					});
			break;
		case LocalCollection::DirStatus::RootPath:
			DirCollection_->setText (tr ("Remove from collection..."));
			DirCollection_->setEnabled (true);
			DirCollectionConn_ = connect (DirCollection_,
					&QAction::triggered,
					this,
					[this]
					{
						const auto& index = Ui_.FSTree_->currentIndex ();
						const auto& path = FSModel_->fileInfo (index).absoluteFilePath ();
						Core::Instance ().GetLocalCollection ()->Unscan (path);
					});
			break;
		case LocalCollection::DirStatus::SubPath:
			DirCollection_->setText (tr ("Already in collection"));
			DirCollection_->setEnabled (false);
			break;
		}
	}

	void FSBrowserWidget::LoadFromFSBrowser ()
	{
		const auto& index = Ui_.FSTree_->currentIndex ();
		if (!index.isValid () || !Player_)
			return;

		const auto& fi = FSModel_->fileInfo (index);
		if (fi.isDir ())
		{
			const bool symLinks = XmlSettingsManager::Instance ().property ("FollowSymLinks").toBool ();
			Player_->Enqueue (RecIterate (fi.absoluteFilePath (), symLinks));
		}
		else
			Player_->Enqueue (QStringList { fi.absoluteFilePath () });
	}
}
