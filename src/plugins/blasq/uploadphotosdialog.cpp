/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploadphotosdialog.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include <util/util.h>
#include "interfaces/blasq/iaccount.h"
#include "interfaces/blasq/isupportuploads.h"
#include "selectalbumdialog.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blasq
{
	namespace
	{
		enum Column
		{
			ThePhoto,
			PhotoSize,
			PhotoDesc
		};
	}

	UploadPhotosDialog::UploadPhotosDialog (QObject *accObj, QWidget *parent)
	: QDialog (parent)
	, AccObj_ (accObj)
	, Acc_ (qobject_cast<IAccount*> (accObj))
	, ISU_ (qobject_cast<ISupportUploads*> (accObj))
	, FilesModel_ (new QStandardItemModel (this))
	{
		FilesModel_->setHorizontalHeaderLabels ({ tr ("Photo"), tr ("Size"), tr ("Description") });
		Ui_.setupUi (this);
		Ui_.PhotosView_->setModel (FilesModel_);

		const auto& fm = fontMetrics ();
		Ui_.PhotosView_->setColumnWidth (Column::ThePhoto,
				Ui_.PhotosView_->iconSize ().width () + fm.horizontalAdvance (" typical image name "));
		Ui_.PhotosView_->setColumnWidth (Column::PhotoSize, fm.horizontalAdvance ("  999.999 KiB  "));

		if (!ISU_->HasUploadFeature (ISupportUploads::Feature::SupportsDescriptions))
			Ui_.PhotosView_->hideColumn (Column::PhotoDesc);

		validate ();
	}

	QModelIndex UploadPhotosDialog::GetSelectedCollection () const
	{
		return SelectedCollection_;
	}

	void UploadPhotosDialog::SetSelectedCollection (const QModelIndex& index)
	{
		SelectedCollection_ = index;
		Ui_.AlbumName_->setText (index.data (CollectionRole::Name).toString ());
		validate ();
	}

	QList<UploadItem> UploadPhotosDialog::GetSelectedFiles () const
	{
		QList<UploadItem> items;
		for (int i = 0, rc = FilesModel_->rowCount (); i < rc; ++i)
			items.append ({
					FilesModel_->index (i, Column::ThePhoto).data (Role::Filepath).toString (),
					FilesModel_->index (i, Column::PhotoDesc).data ().toString ()
				});
		return items;
	}

	void UploadPhotosDialog::SetFiles (const QList<UploadItem>& items)
	{
		for (const auto& item : items)
			AppendPhotoItem (item);
		validate ();
	}

	void UploadPhotosDialog::LockFiles ()
	{
		Ui_.AddPhotoButton_->setEnabled (false);
		Ui_.RemovePhotoButton_->setEnabled (false);
	}

	void UploadPhotosDialog::on_SelectAlbumButton__released ()
	{
		SelectAlbumDialog dia (Acc_);
		if (dia.exec () != QDialog::Accepted)
			return;

		SetSelectedCollection (dia.GetSelectedCollection ());
	}

	void UploadPhotosDialog::AppendPhotoItem (const UploadItem& uploadItem)
	{
		const QPixmap orig { uploadItem.FilePath_ };
		const auto& scaled = orig.scaled (Ui_.PhotosView_->iconSize (),
				Qt::KeepAspectRatio, Qt::SmoothTransformation);

		const QFileInfo finfo { uploadItem.FilePath_ };

		auto item = new QStandardItem { scaled, finfo.fileName () };
		item->setEditable (false);
		item->setData (uploadItem.FilePath_, Role::Filepath);

		auto sizeItem = new QStandardItem (Util::MakePrettySize (finfo.size ()));
		sizeItem->setEditable (false);

		FilesModel_->appendRow ({ item, sizeItem, new QStandardItem { uploadItem.Description_ } });
	}

	void UploadPhotosDialog::on_AddPhotoButton__released ()
	{
		const auto& lastUploadDirectory = XmlSettingsManager::Instance ()
				.Property ("LastUploadDirectory", QDir::homePath ()).toString ();
		const auto& filenames = QFileDialog::getOpenFileNames (this,
				tr ("Select photos to upload"),
				lastUploadDirectory,
				tr ("Images (*.jpg *.png *.gif);;All files (*.*)"));

		if (filenames.isEmpty ())
			return;

		const auto& filePath = QFileInfo { filenames.value (0) }.path ();
		XmlSettingsManager::Instance ().setProperty ("LastUploadDirectory", filePath);

		for (const auto& filename : filenames)
			AppendPhotoItem ({ filename, {} });

		validate ();
	}

	void UploadPhotosDialog::on_RemovePhotoButton__released ()
	{
		const auto& rows = Ui_.PhotosView_->selectionModel ()->selectedRows ();
		QList<QStandardItem*> items;
		for (const auto& idx : rows)
			items << FilesModel_->itemFromIndex (idx);

		for (auto item : items)
			FilesModel_->removeRow (item->row ());

		validate ();
	}

	void UploadPhotosDialog::validate ()
	{
		bool valid = true;

		if (!FilesModel_->rowCount ())
			valid = false;

		if (ISU_->HasUploadFeature (ISupportUploads::Feature::RequiresAlbumOnUpload) &&
				!SelectedCollection_.isValid ())
			valid = false;

		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (valid);
	}
}
}
