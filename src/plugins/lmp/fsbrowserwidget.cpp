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

namespace LC
{
namespace LMP
{
	FSBrowserWidget::FSBrowserWidget (QWidget *parent)
	: QWidget (parent)
	, Player_ (0)
	, FSModel_ (new FSModel (this))
	, ColumnsBeenResized_ (false)
	{
		Ui_.setupUi (this);

		FSModel_->setReadOnly (true);
		FSModel_->setRootPath (QDir::rootPath ());
		Ui_.FSTree_->setModel (FSModel_);

		auto addToPlaylist = new QAction (tr ("Add to playlist"), this);
		addToPlaylist->setProperty ("ActionIcon", "list-add");
		connect (addToPlaylist,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromFSBrowser ()));
		Ui_.FSTree_->addAction (addToPlaylist);

		DirCollection_ = new QAction (QString (), this);
		DirCollection_->setProperty ("WatchActionIconChange", true);
		Ui_.FSTree_->addAction (DirCollection_);

		Ui_.FSTree_->addAction (Util::CreateSeparator (this));

		ViewProps_ = new QAction (tr ("Show track properties"), this);
		ViewProps_->setProperty ("ActionIcon", "document-properties");
		connect (ViewProps_,
				SIGNAL (triggered ()),
				this,
				SLOT (viewProps ()));
		Ui_.FSTree_->addAction (ViewProps_);

		connect (Ui_.FSTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleItemSelected (QModelIndex)));

		connect (Core::Instance ().GetLocalCollection (),
				SIGNAL (rootPathsChanged (QStringList)),
				this,
				SLOT (handleCollectionChanged ()));
	}

	void FSBrowserWidget::showEvent (QShowEvent *event)
	{
		QWidget::showEvent (event);

		if (!ColumnsBeenResized_)
		{
			Ui_.FSTree_->setColumnWidth (0, Ui_.FSTree_->width () * 0.6);
			ColumnsBeenResized_ = true;
		}
	}

	void FSBrowserWidget::AssociatePlayer (Player *player)
	{
		Player_ = player;
	}

	void FSBrowserWidget::handleItemSelected (const QModelIndex& index)
	{
		const auto& fi = FSModel_->fileInfo (index);
		ViewProps_->setEnabled (fi.isFile ());

		const auto& path = fi.absoluteFilePath ();

		disconnect (DirCollection_,
				0,
				this,
				0);

		switch (Core::Instance ().GetLocalCollection ()->GetDirStatus (path))
		{
		case LocalCollection::DirStatus::None:
			DirCollection_->setText (tr ("Add to collection..."));
			DirCollection_->setEnabled (true);
			connect (DirCollection_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleAddToCollection ()));
			break;
		case LocalCollection::DirStatus::RootPath:
			DirCollection_->setText (tr ("Remove from collection..."));
			DirCollection_->setEnabled (true);
			connect (DirCollection_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleRemoveFromCollection ()));
			break;
		case LocalCollection::DirStatus::SubPath:
			DirCollection_->setText (tr ("Already in collection"));
			DirCollection_->setEnabled (false);
			break;
		}
	}

	void FSBrowserWidget::handleCollectionChanged ()
	{
		handleItemSelected (Ui_.FSTree_->currentIndex ());
	}

	void FSBrowserWidget::handleAddToCollection ()
	{
		const auto& index = Ui_.FSTree_->currentIndex ();
		const auto& path = FSModel_->fileInfo (index).absoluteFilePath ();

		Core::Instance ().GetLocalCollection ()->Scan (path);
	}

	void FSBrowserWidget::handleRemoveFromCollection ()
	{
		const auto& index = Ui_.FSTree_->currentIndex ();
		const auto& path = FSModel_->fileInfo (index).absoluteFilePath ();

		Core::Instance ().GetLocalCollection ()->Unscan (path);
	}

	void FSBrowserWidget::loadFromFSBrowser ()
	{
		if (!Player_)
			return;

		const QModelIndex& index = Ui_.FSTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const QFileInfo& fi = FSModel_->fileInfo (index);

		if (fi.isDir ())
		{
			const bool symLinks = XmlSettingsManager::Instance ()
					.property ("FollowSymLinks").toBool ();
			Player_->Enqueue (RecIterate (fi.absoluteFilePath (), symLinks));
		}
		else
			Player_->Enqueue (QStringList (fi.absoluteFilePath ()));
	}

	void FSBrowserWidget::viewProps ()
	{
		const QModelIndex& index = Ui_.FSTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const QFileInfo& fi = FSModel_->fileInfo (index);

		AudioPropsWidget::MakeDialog ()->SetProps (fi.absoluteFilePath ());
	}
}
}
