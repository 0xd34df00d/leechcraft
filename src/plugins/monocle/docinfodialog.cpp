/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "docinfodialog.h"
#include <QStandardItemModel>
#include <util/sll/qtutil.h>
#include <util/threads/coro/context.h>
#include "interfaces/monocle/idocument.h"
#include "interfaces/monocle/ihavefontinfo.h"

namespace LC::Monocle
{
	DocInfoDialog::DocInfoDialog (IDocument& doc, QWidget *parent)
	: QDialog { parent }
	, FontsModel_ { new QStandardItemModel { this } }
	{
		Ui_.setupUi (this);

		Ui_.FontsView_->setModel (FontsModel_);

		const auto& url = doc.GetDocURL ();
		Ui_.FilePath_->setText (url.isLocalFile () ? url.toLocalFile () : url.toString ());

		const auto& info = doc.GetDocumentInfo ();
		Ui_.Title_->setText (info.Title_);
		Ui_.Subject_->setText (info.Subject_);
		Ui_.Author_->setText (info.Author_);
		Ui_.Genres_->setText (info.Genres_.join ("; "_qs));
		Ui_.Keywords_->setText (info.Keywords_.join ("; "_qs));
		Ui_.Date_->setText (info.Date_.toString ());

		const auto ihf = qobject_cast<IHaveFontInfo*> (doc.GetQObject ());
		Ui_.TabWidget_->setTabEnabled (Ui_.TabWidget_->indexOf (Ui_.FontsTab_), ihf);

		if (ihf)
			[] (IHaveFontInfo *ihf, DocInfoDialog *pThis) -> Util::ContextTask<>
			{
				co_await Util::AddContextObject { *pThis };
				pThis->HandleFontsInfo (co_await ihf->RequestFontInfos ());
			} (ihf, this);
	}

	void DocInfoDialog::HandleFontsInfo (const QList<FontInfo>& infos)
	{
		FontsModel_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Path") });
		for (const auto& info : infos)
		{
			const QList<QStandardItem*> row
			{
				new QStandardItem { info.FontName_ },
				new QStandardItem { info.IsEmbedded_ ?
						tr ("embedded") :
						info.LocalPath_ }
			};
			for (auto item : row)
				item->setEditable (false);
			FontsModel_->appendRow (row);
		}
	}
}
