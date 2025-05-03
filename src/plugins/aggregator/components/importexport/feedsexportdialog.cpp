/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "feedsexportdialog.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <util/models/itemsmodel.h>
#include "components/storage/storagebackendmanager.h"
#include "feed.h"

namespace LC::Aggregator
{
	struct FeedsExportDialog::FeedInfo
	{
		IDType_t FeedId_;
		QString Name_;
		QUrl Url_;
		Qt::CheckState CheckState_ = Qt::Checked;
	};

	using FeedsModel_t = Util::ItemsModel<FeedsExportDialog::FeedInfo, Util::ItemsCheckable<&FeedsExportDialog::FeedInfo::CheckState_>>;

	FeedsExportDialog::FeedsExportDialog (QWidget *parent)
	: QDialog { parent }
	, FeedsModel_ { std::make_unique<FeedsModel_t> (
			Util::Field<&FeedInfo::Name_> { tr ("Name") },
			Util::Field<&FeedInfo::Url_> { tr ("URL") }
		)
	}
	{
		Ui_.setupUi (this);
		Ui_.Channels_->setModel (FeedsModel_.get ());
		Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (false);

		connect (Ui_.Browse_,
				&QPushButton::released,
				this,
				&FeedsExportDialog::Browse);
		Browse ();

		connect (Ui_.File_,
				&QLineEdit::textEdited,
				this,
				[this] (const QString& text)
				{
					Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (!text.isEmpty ());
				});
	}

	FeedsExportDialog::~FeedsExportDialog () = default;

	QString FeedsExportDialog::GetDestination () const
	{
		return Ui_.File_->text ();
	}

	QString FeedsExportDialog::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	QString FeedsExportDialog::GetOwner () const
	{
		return Ui_.Owner_->text ();
	}

	QString FeedsExportDialog::GetOwnerEmail () const
	{
		return Ui_.OwnerEmail_->text ();
	}

	QSet<IDType_t> FeedsExportDialog::GetSelectedFeeds () const
	{
		QSet<IDType_t> result;
		for (const auto& item : FeedsModel_->GetItems ())
			if (item.CheckState_ == Qt::Checked)
				result << item.FeedId_;
		return result;
	}

	void FeedsExportDialog::SetFeeds (const channels_shorts_t& channels)
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		QList<FeedInfo> feeds;
		for (const auto& cs : channels)
			{
				const auto& feed = sb->GetFeed (cs.FeedID_);
				feeds.push_back ({ .FeedId_ = cs.FeedID_, .Name_ = cs.Title_, .Url_ = feed.URL_ });
			}
		FeedsModel_->SetItems (std::move (feeds));
	}

	void FeedsExportDialog::Browse ()
	{
		const auto firstPathChoice = Ui_.File_->text ().isEmpty ();
		auto startingPath = firstPathChoice ? QString {} : QFileInfo { Ui_.File_->text () }.path ();
		if (startingPath.isEmpty ())
			startingPath = QDir::homePath () + "/feeds.opml";

		const auto& filename = QFileDialog::getSaveFileName (this,
				tr ("Export as"),
				startingPath,
				tr ("OPML files (*.opml);;"
					"All files (*.*)"));
		if (filename.isEmpty () && firstPathChoice)
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
