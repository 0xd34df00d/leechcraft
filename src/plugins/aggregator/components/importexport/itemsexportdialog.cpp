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
#include <QMessageBox>
#include <interfaces/structures.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/tags/categoryselector.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include "components/models/channelsmodel.h"
#include "components/storage/storagebackendmanager.h"
#include "writefb2.h"
#include "writepdf.h"
#include "itemutils.h"

namespace LC
{
namespace Aggregator
{
	ItemsExportDialog::ItemsExportDialog (ChannelsModel& channelsModel, QWidget *parent)
	: QDialog { parent }
	, ChannelsModel_ { channelsModel }
	{
		Ui_.setupUi (this);

		Ui_.ChannelsTree_->setModel (&channelsModel);

		Selector_ = new Util::CategorySelector (this);
		Selector_->setWindowFlags (Qt::Widget);
		Selector_->SetPossibleSelections ({});
		Ui_.VLayout_->addWidget (Selector_);

		connect (Ui_.ChannelsTree_->selectionModel (),
				&QItemSelectionModel::selectionChanged,
				this,
				&ItemsExportDialog::handleChannelsSelectionChanged);

		connect (this,
				&ItemsExportDialog::accepted,
				this,
				&ItemsExportDialog::handleAccepted);

		on_File__textChanged (QString ());
	}

	PdfConfig ItemsExportDialog::GetPdfConfig (const ExportConfig& exportConfig) const
	{
		const QList<QPageSize::PageSizeId> pageSizes
		{
			QPageSize::A4,
			QPageSize::A5,
			QPageSize::Letter,
		};

		return PdfConfig
		{
			.CommonExport_ = exportConfig,
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

	void ItemsExportDialog::on_Browse__released ()
	{
		const auto& filename = QFileDialog::getSaveFileName (this,
				tr ("Select save file"),
				QDir::homePath () + "/export.fb2",
				tr ("fb2 files (*.fb2);;PDF files (*.pdf);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		Ui_.File_->setText (filename);

		if (filename.endsWith (".pdf"))
			Ui_.ExportFormat_->setCurrentIndex (1);
		else
			Ui_.ExportFormat_->setCurrentIndex (0);
	}

	void ItemsExportDialog::on_File__textChanged (const QString& name)
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (!name.isEmpty ());
	}

	void ItemsExportDialog::on_Name__textEdited ()
	{
		HasBeenTextModified_ = true;
	}

	namespace
	{
		QStringList CatsFromIndexes (const QList<QModelIndex>& indices)
		{
			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

			return Util::ConcatMap (indices,
					[&sb] (const QModelIndex& index)
					{
						const auto channelId = index.data (ChannelRoles::ChannelID).value<IDType_t> ();
						return ItemUtils::GetCategories (sb->GetItems (channelId));
					}).values ();
		}
	}

	void ItemsExportDialog::handleChannelsSelectionChanged ()
	{
		Selector_->SetPossibleSelections (CatsFromIndexes (Ui_.ChannelsTree_->selectionModel ()->selectedRows ()));
		Selector_->SelectAll ();

		if (!HasBeenTextModified_ &&
				Ui_.ChannelsTree_->selectionModel ()->selectedRows ().size () <= 1)
		{
			const QModelIndex& index = Ui_.ChannelsTree_->currentIndex ();
			if (index.isValid ())
				Ui_.Name_->setText (index.sibling (index.row (), 0).data ().toString ());
		}
	}

	void ItemsExportDialog::handleAccepted ()
	{
		QFile file { Ui_.File_->text () };
		if (!file.open (QIODevice::WriteOnly))
		{
			QMessageBox::critical (this,
					MessageBoxTitle,
					tr ("Could not open file %1 for write: %2.")
						.arg (Ui_.File_->text (), file.errorString ()));
			return;
		}

		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		bool unreadOnly = Ui_.UnreadOnly_->checkState () == Qt::Checked;
		const auto& categories = Selector_->GetSelections ();

		QMap<ChannelShort, QList<Item>> items2write;

		for (const auto& row : Ui_.ChannelsTree_->selectionModel ()->selectedRows ())
		{
			const auto& cs = ChannelsModel_.GetChannelForIndex (row);

			const auto& items = sb->GetItems (cs.ChannelID_);

			for (const auto& item : items)
			{
				if (unreadOnly &&
						!item.Unread_)
					continue;

				if (!item.Categories_.isEmpty ())
				{
					const auto suitable = std::any_of (categories.begin (), categories.end (),
							[&item] (const QString& cat) { return item.Categories_.contains (cat); });
					if (!suitable)
						continue;
				}

				if (const auto& fullItem = sb->GetItem (item.ItemID_))
					items2write [cs].prepend (*fullItem);
			}
		}

		const ExportConfig exportConfig
		{
			.Title_ = Ui_.Name_->text (),
		};

		switch (Ui_.ExportFormat_->currentIndex ())
		{
		case 0:
			WriteFB2 (exportConfig, items2write, file);
			break;
		case 1:
			WritePDF (GetPdfConfig (exportConfig), items2write);
			break;
		default:
			qWarning () << "unknown format ID" << Ui_.ExportFormat_->currentIndex ();
			break;
		}

		const auto iem = GetProxyHolder ()->GetEntityManager ();
		iem->HandleEntity (Util::MakeNotification (NotificationTitle,
					tr ("Export complete."),
					Priority::Info));
	}
}
}
