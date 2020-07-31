/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xep0313prefiq.h"
#include "xep0313manager.h"
#include <QDomElement>
#include <QtDebug>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	Xep0313PrefIq::Xep0313PrefIq (QXmppIq::Type type)
	: QXmppIq { type }
	{
	}

	QStringList Xep0313PrefIq::GetAllowed () const
	{
		return Allowed_;
	}

	void Xep0313PrefIq::SetAllowed (const QStringList& allowed)
	{
		Allowed_ = allowed;
	}

	QStringList Xep0313PrefIq::GetForbidden () const
	{
		return Forbidden_;
	}

	void Xep0313PrefIq::SetForbidden (const QStringList& forbidden)
	{
		Forbidden_ = forbidden;
	}

	Xep0313PrefIq::DefaultPolicy Xep0313PrefIq::GetDefaultPolicy () const
	{
		return Policy_;
	}

	void Xep0313PrefIq::SetDefaultPolicy (Xep0313PrefIq::DefaultPolicy policy)
	{
		Policy_ = policy;
	}

	namespace
	{
		Xep0313PrefIq::DefaultPolicy PolicyStr2Policy (const QString& str)
		{
			if (str == "always")
				return Xep0313PrefIq::DefaultPolicy::Always;
			else if (str == "never")
				return Xep0313PrefIq::DefaultPolicy::Never;
			else if (str == "roster")
				return Xep0313PrefIq::DefaultPolicy::Roster;
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown policy"
						<< str;
				return Xep0313PrefIq::DefaultPolicy::Roster;
			}
		}
	}

	void Xep0313PrefIq::parseElementFromChild (const QDomElement& element)
	{
		QXmppIq::parseElementFromChild (element);

		const auto& prefs = element.firstChildElement ("prefs");
		if (prefs.isNull ())
			return;

		Policy_ = PolicyStr2Policy (prefs.attribute ("default").toLower ());

		auto fill = [&prefs] (QStringList& list, const QString& tagName) -> void
		{
			list.clear ();

			auto jid = prefs.firstChildElement (tagName).firstChildElement ("jid");
			while (!jid.isNull ())
			{
				list << jid.text ();
				jid = jid.nextSiblingElement ("jid");
			}
		};

		fill (Allowed_, "always");
		fill (Forbidden_, "never");
	}

	namespace
	{
		QString Policy2Str (const Xep0313PrefIq::DefaultPolicy policy)
		{
			switch (policy)
			{
			case Xep0313PrefIq::DefaultPolicy::Always:
				return "always";
			case Xep0313PrefIq::DefaultPolicy::Never:
				return "never";
			case Xep0313PrefIq::DefaultPolicy::Roster:
				return "roster";
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown policy"
					<< static_cast<int> (policy);
			return "roster";
		}
	}

	void Xep0313PrefIq::toXmlElementFromChild (QXmlStreamWriter *writer) const
	{
		QXmppIq::toXmlElementFromChild (writer);

		writer->writeStartElement ("prefs");
		writer->writeAttribute ("xmlns", Xep0313Manager::GetNsUri ());

		if (type () != QXmppIq::Get)
		{
			writer->writeAttribute ("default", Policy2Str (Policy_));

			auto dump = [writer] (const QStringList& list, const QString& tagName) -> void
			{
				writer->writeStartElement (tagName);

				for (const auto& jid : list)
					writer->writeTextElement ("jid", jid);

				writer->writeEndElement ();
			};

			dump (Allowed_, "always");
			dump (Forbidden_, "never");
		}

		writer->writeEndElement ();
	}
}
}
}
