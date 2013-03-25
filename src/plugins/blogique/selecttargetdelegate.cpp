/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "selecttargetdelegate.h"
#include <QComboBox>
#include <QtDebug>
#include "interfaces/blogique/iaccount.h"
#include "interfaces/blogique/ibloggingplatform.h"
#include "interfaces/blogique/iprofile.h"
#include "submittodialog.h"

namespace LeechCraft
{
namespace Blogique
{
	SelectTargetDelegate::SelectTargetDelegate (SubmitToDialog *dlg, QObject *parent)
	: QItemDelegate (parent)
	, Dlg_ (dlg)
	{
	}

	QWidget* SelectTargetDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
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
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		editor->setGeometry (option.rect);
	}
}
}
