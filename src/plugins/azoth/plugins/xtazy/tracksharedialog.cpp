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

#include "tracksharedialog.h"
#include <QFileInfo>
#include <util/util.h>
#include <interfaces/azoth/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xtazy
{
	TrackShareDialog::TrackShareDialog (const QString& path,
			const QStringList& services, QObject *entryObj, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		auto entry = qobject_cast<ICLEntry*> (entryObj);

		QFileInfo info (path);
		Ui_.FileLabel_->setText (Ui_.FileLabel_->text ()
					.arg (info.fileName ())
					.arg (Util::MakePrettySize (info.size ()))
					.arg (entry->GetEntryName ()));

		Ui_.Services_->addItems (services);
	}

	QString TrackShareDialog::GetVariantName () const
	{
		return Ui_.Services_->currentText ();
	}
}
}
}
