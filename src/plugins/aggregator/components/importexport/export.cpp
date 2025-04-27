/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "export.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include "components/storage/storagebackendmanager.h"
#include "feed.h"

namespace LC
{
namespace Aggregator
{
	namespace
	{
		constexpr auto IDRole = Qt::UserRole;
	}

	Export::Export (const QString& title,
			const QString& exportTitle,
			const QString& choices,
			QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		setWindowTitle (title);
		Title_ = exportTitle;
		Choices_ = choices;
		Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (false);
		on_Browse__released ();
	}
	
	QString Export::GetDestination () const
	{
		return Ui_.File_->text ();
	}
	
	QString Export::GetTitle () const
	{
		return Ui_.Title_->text ();
	}
	
	QString Export::GetOwner () const
	{
		return Ui_.Owner_->text ();
	}
	
	QString Export::GetOwnerEmail () const
	{
		return Ui_.OwnerEmail_->text ();
	}
	
	QSet<IDType_t> Export::GetSelectedFeeds () const
	{
		QSet<IDType_t> result;
		for (int i = 0, items = Ui_.Channels_->topLevelItemCount (); i < items; ++i)
		{
			const auto item = Ui_.Channels_->topLevelItem (i);
			if (item->data (0, Qt::CheckStateRole) == Qt::Checked)
				result << item->data (0, IDRole).value<IDType_t> ();
		}
		return result;
	}
	
	void Export::SetFeeds (const channels_shorts_t& channels)
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		for (const auto& cs : channels)
			{
				const auto& feed = sb->GetFeed (cs.FeedID_);
				const auto item = new QTreeWidgetItem (Ui_.Channels_, { cs.Title_, feed.URL_ });
				item->setData (0, Qt::CheckStateRole, Qt::Checked);
				item->setData (0, IDRole, cs.ChannelID_);
			}
	}
	
	void Export::on_File__textEdited (const QString& text)
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (!text.isEmpty ());
	}
	
	void Export::on_Browse__released ()
	{
		QString startingPath = QFileInfo (Ui_.File_->text ()).path ();
		if (Ui_.File_->text ().isEmpty () ||
				startingPath.isEmpty ())
			startingPath = QDir::homePath () + "/feeds.opml";
	
		QString filename = QFileDialog::getSaveFileName (this,
				Title_,
				startingPath,
				Choices_);
		if (filename.isEmpty ())
		{
			QTimer::singleShot (0,
					this,
					SLOT (reject ()));
			return;
		}
	
		Ui_.File_->setText (filename);
		Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (true);
	}
}
}
