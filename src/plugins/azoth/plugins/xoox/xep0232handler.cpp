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

#include "xep0232handler.h"
#include <QtDebug>
#include <QXmppDataForm.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
namespace XEP0232Handler
{
	const QString SWInfoFormType = "urn:xmpp:dataforms:softwareinfo";

	bool SoftwareInformation::IsNull () const
	{
		return OS_.isEmpty () && OSVer_.isEmpty () &&
				Software_.isEmpty () && SoftwareVer_.isEmpty ();
	}

	SoftwareInformation FromDataForm (const QXmppDataForm& form)
	{
		SoftwareInformation si;
		Q_FOREACH (const QXmppDataForm::Field& f, form.fields ())
		{
			const auto& var = f.key ();
			if (var == "icon")
			{
				const auto& media = f.media ();
				si.IconWidth_ = media.width ();
				si.IconHeight_ = media.height ();
				Q_FOREACH (const auto& pair, media.uris ())
					if (pair.second.startsWith ("http"))
					{
						si.IconURL_ = QUrl::fromEncoded (pair.second.toLatin1 ());
						si.IconType_ = pair.first;
					}
					else if (pair.second.startsWith ("cid"))
						si.IconCID_ = pair.second;
			}
			else if (var == "os")
				si.OS_ = f.value ().toString ();
			else if (var == "os_version")
				si.OSVer_ = f.value ().toString ();
			else if (var == "software")
				si.Software_ = f.value ().toString ();
			else if (var == "software_version")
				si.SoftwareVer_ = f.value ().toString ();
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown field"
						<< var
						<< f.value ();
		}
		return si;
	}

	QXmppDataForm ToDataForm (const SoftwareInformation& si)
	{
		QList<QXmppDataForm::Field> fields;

		QXmppDataForm::Field typeField (QXmppDataForm::Field::HiddenField);
		typeField.setKey ("FORM_TYPE");
		typeField.setValue (SWInfoFormType);
		fields << typeField;

		QXmppDataForm::Field iconField;
		iconField.setKey ("icon");
		QXmppDataForm::Media media (si.IconHeight_, si.IconWidth_);
		QList<QPair<QString, QString>> uris;
		uris << qMakePair<QString, QString> (si.IconType_, si.IconCID_);
		uris << qMakePair<QString, QString> (si.IconType_, si.IconURL_.toEncoded ());
		media.setUris (uris);
		iconField.setMedia (media);

		auto setStr = [&fields] (const QString& key, const QString& val)
		{
			if (!val.isEmpty ())
			{
				QXmppDataForm::Field field;
				field.setKey (key);
				field.setValue (val);
				fields << field;
			}
		};
		setStr ("os", si.OS_);
		setStr ("os_version", si.OSVer_);
		setStr ("software", si.Software_);
		setStr ("software_version", si.SoftwareVer_);

		QXmppDataForm form (QXmppDataForm::Result);
		form.setFields (fields);
		return form;
	}
}
}
}
}
