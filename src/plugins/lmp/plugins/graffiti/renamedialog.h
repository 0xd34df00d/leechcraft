/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/mediainfo.h>
#include <interfaces/lmp/ilmputilproxy.h>
#include "ui_renamedialog.h"

class QStandardItemModel;

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	class RenameDialog : public QDialog
	{
		Q_OBJECT

		const ILMPProxy_ptr Proxy_;

		Ui::RenameDialog Ui_;

		QStandardItemModel *PreviewModel_;

		QList<MediaInfo> Infos_;
		QList<QString> Names_;
	public:
		RenameDialog (ILMPProxy_ptr, QWidget* = 0);

		void SetInfos (const QList<MediaInfo>&);
	private:
		QList<QPair<QString, QString>> GetRenames () const;
	public slots:
		void accept ();
	private slots:
		void updatePreview ();
	};
}
}
}
