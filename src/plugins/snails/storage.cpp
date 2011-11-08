/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "storage.h"
#include <QFile>
#include <QApplication>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "account.h"

namespace LeechCraft
{
namespace Snails
{
	Storage::Storage (QObject *parent)
	: QObject (parent)
	, Settings_ (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Snails_Storage")
	{
		SDir_ = Util::CreateIfNotExists ("snails/storage");
	}

	void Storage::SaveMessages (Account *acc, const QList<Message_ptr>& msgs)
	{
		const QByteArray id = acc->GetID ().toHex ();

		QDir dir = SDir_;
		if (!dir.exists (id))
			dir.mkdir (id);
		if (!dir.cd (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cd into"
					<< dir.filePath (id);
			return;
		}

		Q_FOREACH (Message_ptr msg, msgs)
		{
			if (msg->GetID ().isEmpty ())
				continue;

			const QString dirName = msg->GetID ().toHex ().left (2);

			QDir msgDir = dir;
			if (!dir.exists (dirName))
				msgDir.mkdir (dirName);
			if (!msgDir.cd (dirName))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd into"
						<< msgDir.filePath (dirName);
				continue;
			}

			QFile file (msgDir.filePath (msg->GetID ().toHex ()));
			file.open (QIODevice::WriteOnly);
			file.write (qCompress (msg->Serialize (), 9));

			qApp->processEvents ();
		}
	}
}
}
