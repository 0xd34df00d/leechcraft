/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlgenerator.h"
#include <QXmlStreamWriter>
#include <QFileInfo>

namespace LC
{
namespace Dolozhee
{
	QByteArray XMLGenerator::RegisterUser (const QString& login, const QString& pass,
			const QString& email, const QString& firstname, const QString& lastname) const
	{
		QByteArray result;

		QXmlStreamWriter w (&result);
		w.writeStartDocument ();
		w.writeStartElement ("user");
		w.writeTextElement ("login", login);
		w.writeTextElement ("password", pass);
		w.writeTextElement ("mail", email);
		w.writeTextElement ("firstname", firstname);
		w.writeTextElement ("lastname", lastname);
		w.writeEndDocument ();

		return result;
	}

	QByteArray XMLGenerator::CreateIssue (const QString& title,
			QString desc, int category,
			ReportTypePage::Type type, ReportTypePage::Priority prio,
			const QList<FileInfo>& files) const
	{
		desc.remove ("\r");

		QByteArray result;

		QXmlStreamWriter w (&result);
		w.writeStartDocument ();
		w.writeStartElement ("issue");
		w.writeTextElement ("subject", title);
		w.writeTextElement ("description", desc);
		w.writeTextElement ("project_id", "1");
		w.writeTextElement ("priority_id", QString::number (static_cast<int> (prio) + 3));
		if (category >= 0)
			w.writeTextElement ("category_id", QString::number (category));
		switch (type)
		{
		case ReportTypePage::Type::Bug:
			w.writeTextElement ("tracker_id", "1");
			break;
		case ReportTypePage::Type::Feature:
			w.writeTextElement ("tracker_id", "2");
			break;
		}

		if (!files.isEmpty ())
		{
			w.writeStartElement ("uploads");
			w.writeAttribute ("type", "array");
			for (const auto& file : files)
			{
				w.writeStartElement ("upload");
				w.writeTextElement ("token", file.Token_);
				w.writeTextElement ("filename", QFileInfo (file.Name_).fileName ());
				w.writeTextElement ("description", file.Description_);
				w.writeTextElement ("content_type", file.Mime_);
				w.writeEndElement ();
			}
			w.writeEndElement ();
		}

		w.writeEndDocument ();

		return result;
	}
}
}
