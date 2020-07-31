/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xep0232handler.h"
#include <algorithm>
#include <QSize>
#include <QtDebug>
#include <QXmppDataForm.h>

namespace LC::Azoth::Xoox::XEP0232Handler
{
	const QString SWInfoFormType = "urn:xmpp:dataforms:softwareinfo";

	bool SoftwareInformation::IsNull () const
	{
		return OS_.isEmpty () && OSVer_.isEmpty () &&
				Software_.isEmpty () && SoftwareVer_.isEmpty ();
	}

	SoftwareInformation FromDataForm (const QXmppDataForm& form)
	{
		if (form.isNull ())
			return SoftwareInformation ();

		const auto& fields = form.fields ();
		auto pos = std::find_if (fields.begin (), fields.end (),
				[] (const auto& field) { return field.key () == "FORM_TYPE"; });
		if (pos == fields.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no FORM_TYPE"
					<< form.title ();
			return SoftwareInformation ();
		}

		if (pos->value () != SWInfoFormType)
			return SoftwareInformation ();

		SoftwareInformation si;
		for (const auto& f : fields)
		{
			const auto& var = f.key ();
			if (var == "icon")
			{
				si.IconSize_ = f.mediaSize ();
				for (const auto& source : f.mediaSources ())
				{
					const auto& uri = source.uri ();
					if (uri.scheme () == "http")
					{
						si.IconURL_ = uri;
						si.IconType_ = source.contentType ();
					}
					else if (uri.scheme () == "cid")
						si.IconCID_ = uri.toEncoded ();
				}
			}
			else if (var == "os")
				si.OS_ = f.value ().toString ();
			else if (var == "os_version")
				si.OSVer_ = f.value ().toString ();
			else if (var == "software")
				si.Software_ = f.value ().toString ();
			else if (var == "software_version")
				si.SoftwareVer_ = f.value ().toString ();
			else if (var != "FORM_TYPE")
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

		if (si.IconURL_.isValid ())
		{
			QXmppDataForm::Field iconField;
			iconField.setKey ("icon");

			iconField.setMediaSize (si.IconSize_);

			QVector<QXmppDataForm::MediaSource> sources;
			if (!si.IconCID_.isEmpty ())
				sources.append ({ QUrl::fromEncoded (si.IconCID_), si.IconType_ });
			sources.append ({ si.IconURL_, si.IconType_ });
			iconField.setMediaSources (sources);
			iconField.setValue (si.IconURL_.toEncoded ());

			fields << iconField;
		}

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
