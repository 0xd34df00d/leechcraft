/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "converteddoccleaner.h"
#include <QFile>
#include <QUrl>
#include <QtDebug>

namespace LC
{
namespace Monocle
{
	ConvertedDocCleaner::ConvertedDocCleaner (const IDocument_ptr& doc)
	: Path_ { doc->GetDocURL ().toLocalFile () }
	{
		connect (doc->GetQObject (),
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleDestroyed ()));
	}

	void ConvertedDocCleaner::handleDestroyed ()
	{
		qDebug () << Q_FUNC_INFO
				<< "removing"
				<< Path_;
		QFile::remove (Path_);
		deleteLater ();
	}
}
}
