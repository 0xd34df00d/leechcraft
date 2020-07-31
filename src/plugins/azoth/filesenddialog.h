/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_filesenddialog.h"

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	class FileSendDialog : public QDialog
	{
		Q_OBJECT

		Ui::FileSendDialog Ui_;
		ICLEntry *Entry_;
		const QString EntryVariant_;
		bool AccSupportsFT_;

		struct SharerInfo
		{
			QObject *Sharer_;
			QString Service_;
		};
		QMap<int, SharerInfo> Pos2Sharer_;
	public:
		FileSendDialog (ICLEntry*, const QString& = QString (), QWidget* = 0);
	private:
		void FillSharers ();
		void SendSharer (const SharerInfo&);
		void SendProto ();
	private slots:
		void send ();
		void on_FileBrowse__released ();
	};
}
}
