/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "playlistwidget.h"
#include <QToolBar>
#include <QFileDialog>
#include <QDockWidget>
#include <QTextStream>
#include <QStandardItemModel>
#include <vlc/vlc.h>
#include <util/util.h>
#include "chooseurldialog.h"
#include "playlistaddmenu.h"
#include "playbackmodemenu.h"

namespace LeechCraft
{
namespace Laure
{
	PlayListWidget::PlayListWidget (QWidget *parent)
	: QWidget (parent)
	, GridLayout_ (new QGridLayout (this))
	, PlayListModel_ (new QStandardItemModel (this))
	, PlayListView_ (new PlayListView (PlayListModel_, this))
	, ActionBar_ (new QToolBar (this))
	{
		setLayout (GridLayout_);
		setVisible (false);
		
		connect (PlayListModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));
		
		GridLayout_->addWidget (PlayListView_, 0, 0);
		
		GridLayout_->addWidget (ActionBar_, 1, 0);
		GridLayout_->setSpacing (0);
		GridLayout_->setMargin (0);
		
		QAction *actionAdd = new QAction (tr ("Add"), this);
		QAction *actionRemove = new QAction (tr ("Remove"), this);
		QAction *actionPlayback = new QAction (tr ("Playback mode"), this);
		QAction *actionExport = new QAction (tr ("Export to m3u"), this);
		
		auto menuAdd = new PlayListAddMenu (this);
		auto menuMode = new PlaybackModeMenu (this);
		
		actionAdd->setProperty ("ActionIcon", "list-add");
		actionRemove->setProperty ("ActionIcon", "list-remove");
		actionExport->setProperty ("ActionIcon", "document-save-as");
		actionPlayback->setProperty ("ActionIcon", "flag-black");
		
		actionAdd->setMenu (menuAdd);
		actionAdd->setMenuRole (QAction::ApplicationSpecificRole);
		
		actionPlayback->setMenu (menuMode);
		actionPlayback->setMenuRole (QAction::ApplicationSpecificRole);
		
		ActionBar_->setToolButtonStyle (Qt::ToolButtonIconOnly);
		ActionBar_->setIconSize (QSize (16, 16));
		ActionBar_->addAction (actionAdd);
		ActionBar_->addAction (actionRemove);
		ActionBar_->addAction (actionPlayback);
		ActionBar_->addAction (actionExport);

		connect (actionRemove,
				SIGNAL (triggered (bool)),
				PlayListView_,
				SLOT (removeSelectedRows ()));
		connect (actionExport,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleExportPlayList ()));
		connect (menuAdd,
				SIGNAL (addItem (QString)),
				this,
				SIGNAL (itemAddedRequest (QString)));
		connect (menuMode,
				SIGNAL (playbackModeChanged (PlaybackMode)),
				this,
				SIGNAL (playbackModeChanged (PlaybackMode)));
		connect (PlayListView_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SIGNAL (doubleClicked ()));
	}
	
	void PlayListWidget::Init (std::shared_ptr<VLCWrapper> wrapper)
	{
		PlayListView_->Init (wrapper);
		VLCWrapper_ = wrapper;
		
		VLCWrapper *w = wrapper.get ();
		connect (this,
				SIGNAL (itemAddedRequest (QString)),
				w,
				SLOT (addRow (QString)));

		connect (this,
				SIGNAL (playbackModeChanged (PlaybackMode)),
				w,
				SLOT (setPlaybackMode (PlaybackMode)));
		connect (w,
				SIGNAL (itemAdded (MediaMeta, QString)),
				this,
				SLOT (handleItemAdded (MediaMeta, QString)));
		connect (w,
				SIGNAL (itemPlayed (int)),
				this,
				SLOT (handleItemPlayed (int)));
		connect (this,
				SIGNAL (metaChangedRequest (libvlc_meta_t, QString, int)),
				w,
				SLOT (setMeta (libvlc_meta_t, QString, int)));
	}
	
	namespace
	{
		int MetaType (int row)
		{
			switch (row)
			{
			case 1:
				return libvlc_meta_Artist;
			case 2:
				return libvlc_meta_Title;
			case 3:
				return libvlc_meta_Album;
			case 4:
				return libvlc_meta_Genre;
			case 5:
				return libvlc_meta_Date;
			default:
				return -1;
			}
		}
	}
	
	void PlayListWidget::handleItemChanged (QStandardItem *item)
	{
		int type = MetaType (item->column ());
		if (type == -1)
			return;
		
		emit metaChangedRequest (static_cast<libvlc_meta_t> (type),
				item->data (Qt::DisplayRole).toString ().toUtf8 (),
				item->row ());
	}
	
	void PlayListWidget::handleItemPlayed (int row)
	{
		PlayListView_->selectRow (row);
		PlayListView_->MarkPlayingItem (row);
	}
	
	void PlayListWidget::handleItemAdded (const MediaMeta& meta,
			const QString& fileName)
	{
		PlayListView_->AddItem (meta, fileName);
	}
	
	void PlayListWidget::handleExportPlayList ()
	{
		const QString& fileName = QFileDialog::getSaveFileName (this,
				tr ("Save to playlist"), QDir::homePath (),
				"*.m3u");
		if (fileName.isEmpty ())
			return;
		
		QFile file (fileName);
		if (!file.open (QIODevice::WriteOnly | QIODevice::Text))
		{
			emit gotEntity (Util::MakeNotification ("Laure",
					tr ("Can't export %1")
							.arg (fileName),
					PInfo_));
			return;
		}
		QTextStream out (&file);
		out << "#EXTM3U\n";
		
		for (int i = 0, c = PlayListModel_->rowCount (); i < c; ++i)
			out << PlayListModel_->data (PlayListModel_->index (i, 0))
					.toString () << '\n';
	}
}
}
