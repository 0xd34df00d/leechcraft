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

#include "xmlgenerator.h"
#include <QXmlStreamWriter>

namespace LeechCraft
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
			const QString& desc, ReportTypePage::Type type) const
	{
		QByteArray result;

		QXmlStreamWriter w (&result);
		w.writeStartDocument ();
		w.writeStartElement ("issue");
		w.writeTextElement ("subject", title);
		w.writeTextElement ("description", desc);
		w.writeTextElement ("project_id", "1");
		w.writeTextElement ("priority_id", "4");
		switch (type)
		{
		case ReportTypePage::Type::Bug:
			w.writeTextElement ("tracker_id", "1");
			break;
		case ReportTypePage::Type::Feature:
			w.writeTextElement ("tracker_id", "2");
			break;
		}
		w.writeEndDocument ();

		return result;
	}
}
}
