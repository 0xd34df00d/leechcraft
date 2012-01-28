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

#include "passutils.h"
#include <QString>
#include <QObject>
#include <QInputDialog>
#include "util.h"
#include "interfaces/structures.h"

namespace LeechCraft
{
namespace Util
{
	namespace
	{
		QString GetPasswordHelper (const QString& key, QObject *emitter)
		{
			QList<QVariant> keys;
			keys << key;
			const auto& result = Util::GetPersistentData (keys, emitter);
			if (result.size () != 1)
			{
				qWarning () << Q_FUNC_INFO
						<< "incorrect result size for key"
						<< key
						<< "; result:"
						<< result;
				return QString ();
			}

			const auto& strVarList = result.at (0).toList ();
			if (strVarList.isEmpty () ||
					!strVarList.at (0).canConvert<QString> ())
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid string variant list"
						<< strVarList
						<< "for key"
						<< key;
				return QString ();
			}

			return strVarList.at (0).toString ();
		}
	}

	QString GetPassword (const QString& key, const QString& diaText,
			QObject *emitter, bool useStored)
	{
		if (useStored)
		{
			const QString& result = GetPasswordHelper (key, emitter);
			if (!result.isNull ())
				return result;
		}

		QString result = QInputDialog::getText (0,
				"LeechCraft",
				diaText,
				QLineEdit::Password);
		if (!result.isNull ())
			SavePassword (result, key, emitter);
		return result;
	}

	void SavePassword (const QString& password, const QString& key,
			QObject *emitter)
	{
		QList<QVariant> keys;
		keys << key;

		QList<QVariant> passwordVar;
		passwordVar << password;
		QList<QVariant> values;
		values << QVariant (passwordVar);

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-save");
		e.Additional_ ["Values"] = values;
		e.Additional_ ["Overwrite"] = true;

		QMetaObject::invokeMethod (emitter, "gotEntity", Q_ARG (LeechCraft::Entity, e));
	}
}
}
