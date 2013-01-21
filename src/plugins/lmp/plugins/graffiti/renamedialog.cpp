/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "renamedialog.h"
#include <QStandardItemModel>

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	RenameDialog::RenameDialog (ILMPProxy_Ptr proxy, QWidget *parent)
	: QDialog (parent)
	, Proxy_ (proxy)
	, Getters_ (proxy->GetSubstGetters ())
	, PreviewModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		const auto& helpText = tr ("The following variables are allowed in the pattern: %1.")
				.arg (QStringList (Getters_.keys ()).join ("; "));
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

		for (const auto& info : infos)
		{
			auto sourceItem = new QStandardItem;
			sourceItem->setText (QFileInfo (info.LocalPath_).fileName ());
			PreviewModel_->appendRow ({ sourceItem, new QStandardItem });
		}

		updatePreview ();
	}

	void RenameDialog::updatePreview ()
	{
		const auto& pattern = Ui_.Pattern_->currentText ();
		const bool hasExtension = pattern.contains ('.');

		int row = 0;
		for (const auto& info : Infos_)
		{
			auto name = Proxy_->PerformSubstitutions (pattern, info);
			if (!hasExtension)
				name += '.' + QFileInfo (info.LocalPath_).suffix ();

			auto item = PreviewModel_->item (row++, 1);
			item->setText (name);
		}
	}
}
}
}
