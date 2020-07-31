/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tracksharedialog.h"
#include <QFileInfo>
#include <util/util.h>
#include <interfaces/azoth/iclentry.h>

namespace LC
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
