/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemsexportdialog.h"
#include <algorithm>
#include <QFileDialog>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <util/gui/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include "components/models/channelsmodel.h"
#include "components/storage/storagebackendmanager.h"
#include "channelsexportmodel.h"
#include "exportutils.h"
#include "types.h"

namespace LC::Aggregator
{
	ItemsExportDialog::ItemsExportDialog (ChannelsModel& model, QWidget *parent)
	: QDialog { parent }
	, ChannelsModel_ { std::make_unique<ChannelsExportModel> (ChannelID) }
	{
		ChannelsModel_->setSourceModel (&model);

		Ui_.setupUi (this);

		setWindowIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());

		ManageLastPath ({
					{ this, &ItemsExportDialog::GetFilename, &ItemsExportDialog::SetFilename },
					"ItemsExportLastPath",
					QDir::homePath () + "/export.fb2",
					*this
				});

		Ui_.CategoriesSelector_->setMinimumHeight (0);
		Ui_.CategoriesSelector_->setWindowFlags (Qt::Widget);

		Ui_.ChannelsTree_->setModel (&*ChannelsModel_);
		Ui_.ChannelsTree_->header ()->resizeSections (QHeaderView::ResizeToContents);
		connect (&*ChannelsModel_,
				&Util::CheckableProxyModel<IDType_t>::selectionChanged,
				this,
				&ItemsExportDialog::UpdateCategories);
		connect (Ui_.UnreadOnly_,
				&QCheckBox::toggled,
				this,
				&ItemsExportDialog::UpdateCategories);
		UpdateCategories ();

		connect (Ui_.SelectAll_,
				&QPushButton::released,
				ChannelsModel_.get (),
				&Util::CheckableProxyModel<IDType_t>::CheckAll);
		connect (Ui_.SelectNone_,
				&QPushButton::released,
				ChannelsModel_.get (),
				&Util::CheckableProxyModel<IDType_t>::CheckNone);

		connect (Ui_.Browse_,
				&QPushButton::released,
				this,
				&ItemsExportDialog::Browse);
		connect (Ui_.File_,
				&QLineEdit::textEdited,
				this,
				&ItemsExportDialog::CheckDialogAcceptable);

		connect (Ui_.Title_,
				&QLineEdit::textEdited,
				this,
				[this] { TitleBeenModified_ = true; });

		CheckDialogAcceptable ();
	}

	ItemsExportDialog::~ItemsExportDialog () = default;

	QString ItemsExportDialog::GetFilename () const
	{
		return Ui_.File_->text () + '.' + Ui_.ExportFormat_->currentText ();
	}

	ItemsExportFormat ItemsExportDialog::GetFormat () const
	{
		const auto& title = Ui_.Title_->text ();

		switch (Ui_.ExportFormat_->currentIndex ())
		{
		case 0:
			return Fb2Config { .Title_ = title };
		case 1:
		{
			const QList pageSizes
			{
				QPageSize::A4,
				QPageSize::A5,
				QPageSize::Letter,
			};

			return PdfConfig
			{
				.Title_ = title,
				.Font_ = Ui_.PDFFont_->currentFont (),
				.FontSize_ = Ui_.PDFFontSize_->value (),
				.Margins_ =
				{
					Ui_.LeftMargin_->value (),
					Ui_.TopMargin_->value (),
					Ui_.RightMargin_->value (),
					Ui_.BottomMargin_->value ()
				},
				.PageSize_ = { pageSizes.value (Ui_.PageSizeBox_->currentIndex (), QPageSize::A4) },
				.Filename_ = Ui_.File_->text (),
			};
		}
		default:
			qWarning () << "unknown export type" << Ui_.ExportFormat_->currentIndex ();
			return {};
		}
	}

	ItemsExportDialog::ItemsExportInfo ItemsExportDialog::GetItemsExportInfo () const
	{
		return
		{
			.Channels_ = ChannelsModel_->GetChecked (),
			.Categories_ = Ui_.CategoriesSelector_->GetSelections (),
			.UnreadOnly_ = Ui_.UnreadOnly_->checkState () == Qt::Checked,
		};
	}

	bool ItemsExportDialog::Browse ()
	{
		const auto& filename = QFileDialog::getSaveFileName (this,
				tr ("Select save file"),
				GetFilename (),
				Util::MakeFileDialogFilter ({ { tr ("fb2 files"), "fb2"_ql }, { tr ("PDF files"), "pdf"_ql } }));
		if (filename.isEmpty ())
			return false;

		SetFilename (filename);
		CheckDialogAcceptable ();
		return true;
	}

	void ItemsExportDialog::SetFilename (const QString& filename)
	{
		const QFileInfo fi { filename };
		const auto& basePath = fi.path () + '/' + fi.completeBaseName ();
		const auto& suffix = fi.suffix ();
		Ui_.File_->setText (basePath);
		if (const auto fmtIdx = Ui_.ExportFormat_->findText (suffix, Qt::MatchExactly);
			fmtIdx != -1)
			Ui_.ExportFormat_->setCurrentIndex (fmtIdx);
	}

	void ItemsExportDialog::CheckDialogAcceptable ()
	{
		auto isAcceptable = [this]
		{
			const auto& filename = Ui_.File_->text ();
			if (filename.isEmpty ())
				return false;

			const QFileInfo fi { filename };
			if (fi.isRelative ())
				return false;
			return QFileInfo { fi.path () }.isWritable ();
		};

		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (isAcceptable ());
	}

	namespace
	{
		QStringList CatsFromIndexes (const QSet<IDType_t>& channels, bool unreadOnly)
		{
			using enum StorageBackend::ReadStatus;
			const auto readStatus = unreadOnly ? Unread : All;

			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			return Util::ConcatMap (QList<IDType_t> { channels.begin (), channels.end () },
					[&] (IDType_t channelId)
					{
						return sb->GetItemsCategories (channelId, readStatus);
					}).values ();
		}
	}

	void ItemsExportDialog::UpdateCategories ()
	{
		const auto unreadOnly = Ui_.UnreadOnly_->checkState () == Qt::Checked;
		const auto& checked = ChannelsModel_->GetChecked ();
		Ui_.CategoriesSelector_->SetPossibleSelections (CatsFromIndexes (checked, unreadOnly));
		Ui_.CategoriesSelector_->SelectAll ();

		if (!TitleBeenModified_ && checked.size () == 1)
		{
			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			Ui_.Title_->setText (sb->GetChannel (*checked.begin ()).Title_);
		}
	}
}
