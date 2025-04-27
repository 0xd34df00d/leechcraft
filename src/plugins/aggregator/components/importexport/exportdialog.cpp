/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "exportdialog.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include "components/storage/storagebackendmanager.h"
#include "feed.h"

namespace LC::Aggregator
{
	namespace
	{
		constexpr auto IDRole = Qt::UserRole;
	}

	ExportDialog::ExportDialog (const QString& title,
			const QString& exportTitle,
			const QString& choices,
			QWidget *parent)
	: QDialog (parent)
	, Title_ { exportTitle }
	, Choices_ { choices }
	{
		Ui_.setupUi (this);
		setWindowTitle (title);
		Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (false);

		connect (Ui_.Browse_,
				&QPushButton::released,
				this,
				&ExportDialog::Browse);
		Browse ();

		connect (Ui_.File_,
				&QLineEdit::textEdited,
				this,
				[this] (const QString& text)
				{
					Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (!text.isEmpty ());
				});
	}
	
	QString ExportDialog::GetDestination () const
	{
		return Ui_.File_->text ();
	}
	
	QString ExportDialog::GetTitle () const
	{
		return Ui_.Title_->text ();
	}
	
	QString ExportDialog::GetOwner () const
	{
		return Ui_.Owner_->text ();
	}
	
	QString ExportDialog::GetOwnerEmail () const
	{
		return Ui_.OwnerEmail_->text ();
	}
	
	QSet<IDType_t> ExportDialog::GetSelectedFeeds () const
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
	
	void ExportDialog::SetFeeds (const channels_shorts_t& channels)
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
	
	void ExportDialog::Browse ()
	{
		auto startingPath = QFileInfo { Ui_.File_->text () }.path ();
		if (Ui_.File_->text ().isEmpty () || startingPath.isEmpty ())
			startingPath = QDir::homePath () + "/feeds.opml";
	
		const auto& filename = QFileDialog::getSaveFileName (this,
				Title_,
				startingPath,
				Choices_);
		if (filename.isEmpty ())
		{
			QTimer::singleShot (0,
					this,
					&QDialog::reject);
			return;
		}
	
		Ui_.File_->setText (filename);
		Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (true);
	}
}
