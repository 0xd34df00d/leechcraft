/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "selecttargetdelegate.h"
#include <QComboBox>
#include <QtDebug>
#include "interfaces/blogique/iaccount.h"
#include "interfaces/blogique/ibloggingplatform.h"
#include "interfaces/blogique/iprofile.h"
#include "submittodialog.h"

namespace LC
{
namespace Blogique
{
	SelectTargetDelegate::SelectTargetDelegate (SubmitToDialog *dlg, QObject *parent)
	: QItemDelegate (parent)
	, Dlg_ (dlg)
	{
	}

	QWidget* SelectTargetDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem&, const QModelIndex& index) const
	{
		QComboBox *box = new QComboBox (parent);
		IAccount *acc = Dlg_->GetAccountFromIndex (index.sibling (index.row (),
				SubmitToDialog::Account));
		if (!acc)
			return box;
		auto ibp = qobject_cast<IBloggingPlatform*> (acc->GetParentBloggingPlatform ());
		if (!ibp)
			return box;

		if (ibp->GetFeatures () & IBloggingPlatform::BPFSelectablePostDestination)
		{
			auto profile = qobject_cast<IProfile*> (acc->GetProfile ());
			if (!profile)
				box->addItem (tr ("<Default>"));
			else
				for (const auto& pair : profile->GetPostingTargets ())
					box->addItem (pair.first, pair.second);
		}
		else
			box->addItem (tr ("<Default>"));

		box->setCurrentIndex (0);
		Dlg_->GetModel ()->setData (index, box->currentText (), TargetRole);

		return box;
	}

	void SelectTargetDelegate::setEditorData (QWidget *editor,
			const QModelIndex& index) const
	{
		auto box = static_cast<QComboBox*> (editor);
		QString targetText = index.data (TargetRole).toString ();
		box->setCurrentIndex (box->findText (targetText, Qt::MatchExactly));
	}

	void SelectTargetDelegate::setModelData (QWidget *editor,
			QAbstractItemModel *model, const QModelIndex& index) const
	{
		auto box = static_cast<QComboBox*> (editor);
		model->setData (index, box->currentText (), TargetRole);
	}

	void SelectTargetDelegate::updateEditorGeometry (QWidget *editor,
			const QStyleOptionViewItem& option, const QModelIndex&) const
	{
		editor->setGeometry (option.rect);
	}
}
}
