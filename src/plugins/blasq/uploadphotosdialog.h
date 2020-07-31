/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_uploadphotosdialog.h"

class QStandardItemModel;

namespace LC
{
namespace Blasq
{
	class IAccount;
	class ISupportUploads;
	struct UploadItem;

	class UploadPhotosDialog : public QDialog
	{
		Q_OBJECT

		Ui::UploadPhotosDialog Ui_;

		QObject * const AccObj_;
		IAccount * const Acc_;
		ISupportUploads * const ISU_;

		QStandardItemModel * const FilesModel_;
		QModelIndex SelectedCollection_;

		enum Role
		{
			Filepath = Qt::UserRole + 1
		};
	public:
		UploadPhotosDialog (QObject *accObj, QWidget* = 0);

		QModelIndex GetSelectedCollection () const;
		void SetSelectedCollection (const QModelIndex&);

		QList<UploadItem> GetSelectedFiles () const;
		void SetFiles (const QList<UploadItem>&);
		void LockFiles ();
	private:
		void AppendPhotoItem (const UploadItem&);
	private slots:
		void on_SelectAlbumButton__released ();

		void on_AddPhotoButton__released ();
		void on_RemovePhotoButton__released ();

		void validate ();
	};
}
}
