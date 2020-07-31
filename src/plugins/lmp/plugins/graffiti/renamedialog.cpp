/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "renamedialog.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDir>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/util.h>
#include <util/lmp/util.h>

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	RenameDialog::RenameDialog (ILMPProxy_ptr proxy, QWidget *parent)
	: QDialog (parent)
	, Proxy_ (proxy)
	, PreviewModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		const auto& helpText = tr ("The following variables are allowed in the pattern: %1.")
				.arg (GetSubstGettersKeys ().join ("; "));
		Ui_.PatternDescLabel_->setText (helpText);

		Ui_.Preview_->setModel (PreviewModel_);

		connect (Ui_.Pattern_,
				SIGNAL (editTextChanged (QString)),
				this,
				SLOT (updatePreview ()));
	}

	void RenameDialog::SetInfos (const QList<MediaInfo>& infos)
	{
		Infos_ = infos;

		PreviewModel_->clear ();
		PreviewModel_->setHorizontalHeaderLabels ({ tr ("Source name"), tr ("Target name") });

		for (const auto& info : Infos_)
		{
			auto sourceItem = new QStandardItem;
			sourceItem->setText (QFileInfo { info.LocalPath_ }.fileName ());
			PreviewModel_->appendRow ({ sourceItem, new QStandardItem });
		}

		updatePreview ();
	}

	QList<QPair<QString, QString>> RenameDialog::GetRenames () const
	{
		QList<QPair<QString, QString>> result;
		for (const auto& pair : Util::Zip (Infos_, Names_))
			if (QFileInfo (pair.first.LocalPath_).fileName () != pair.second)
				result.push_back ({ pair.first.LocalPath_, pair.second });
		return result;
	}

	namespace
	{
		void Rename (const QList<QPair<QString, QString>>& pairs)
		{
			for (const auto& pair : pairs)
			{
				const QFileInfo sourceInfo (pair.first);
				auto sourceDir = sourceInfo.absoluteDir ();
				if (!sourceDir.rename (sourceInfo.fileName (), pair.second))
					qWarning () << Q_FUNC_INFO
							<< "failed to rename"
							<< sourceInfo.fileName ()
							<< "to"
							<< pair.second;
			}
		}
	}

	void RenameDialog::accept ()
	{
		const auto guard = Util::MakeScopeGuard ([this] { QDialog::accept (); });

		const auto& toRename = GetRenames ();
		if (toRename.isEmpty ())
			return;

		if (QMessageBox::question (this,
				"LMP Graffiti",
				tr ("Are you sure you want to rename %n file(s)?", 0, toRename.size ()),
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			Rename (toRename);
	}

	void RenameDialog::updatePreview ()
	{
		Names_ = PerformSubstitutions (Ui_.Pattern_->currentText (), Infos_,
				[this] (int row, const QString& name)
					{ PreviewModel_->item (row, 1)->setText (name); });
	}
}
}
}
