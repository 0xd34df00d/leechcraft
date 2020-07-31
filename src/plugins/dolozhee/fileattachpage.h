/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_fileattachpage.h"

class QStandardItemModel;

namespace LC
{
namespace Dolozhee
{
	class FileAttachPage : public QWizardPage
	{
		Q_OBJECT

		Ui::FileAttachPage Ui_;
		QStandardItemModel *Model_;
	public:
		explicit FileAttachPage (QWidget* = nullptr);

		int nextId () const override;

		void AddFile (const QString&);
		QStringList GetFiles () const;
	private slots:
		void on_AddFile__released ();
		void on_RemoveFile__released ();
	};
}
}
