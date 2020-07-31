/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xep0313reqiq.h"
#include <QDomElement>
#include <QtDebug>
#include <QXmppResultSet.h>
#include <util/sll/util.h>
#include "xep0313manager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	Xep0313ReqIq::Xep0313ReqIq (const QString& jid, const QString& startId,
			int count, Direction dir, const QString& queryId)
	: QXmppIq { QXmppIq::Set }
	, JID_ { jid }
	, ItemId_ { startId }
	, Count_ { count }
	, QueryID_ { queryId }
	, Dir_ { dir }
	{
	}

	void Xep0313ReqIq::parseElementFromChild (const QDomElement& element)
	{
		QXmppIq::parseElementFromChild (element);

		const auto& queryElem = element.firstChildElement ("query");

		QXmppDataForm form;
		form.parse (queryElem.firstChildElement ("x"));
		if (!form.isNull ())
			for (const auto& field : form.fields ())
				if (field.key () == "with")
					JID_ = field.value ().toString ();

		QXmppResultSetQuery q;
		q.parse (queryElem.firstChildElement ("set"));

		Count_ = q.max ();

		Dir_ = Direction::Unspecified;
		if (!q.after ().isNull ())
		{
			ItemId_ = q.after ().toLatin1 ();
			Dir_ = Direction::After;
		}
		else if (!q.before ().isNull ())
		{
			ItemId_ = q.before ().toLatin1 ();
			Dir_ = Direction::Before;
		}
	}

	void Xep0313ReqIq::toXmlElementFromChild (QXmlStreamWriter *writer) const
	{
		QXmppIq::toXmlElementFromChild (writer);

		writer->writeStartElement ("query");
		const auto endGuard = Util::MakeScopeGuard ([writer] { writer->writeEndElement (); });

		writer->writeAttribute ("xmlns", Xep0313Manager::GetNsUri ());

		if (!QueryID_.isEmpty ())
			writer->writeAttribute ("queryid", QueryID_);

		if (JID_.isEmpty () && !Count_ && ItemId_.isEmpty ())
			return;

		if (!JID_.isEmpty ())
		{
			QXmppDataForm::Field formTypeField { QXmppDataForm::Field::HiddenField };
			formTypeField.setKey ("FORM_TYPE");
			formTypeField.setValue (Xep0313Manager::GetNsUri ());

			QXmppDataForm::Field jidField { QXmppDataForm::Field::JidSingleField };
			jidField.setKey ("with");
			jidField.setValue (JID_);

			QXmppDataForm form { QXmppDataForm::Form };
			form.setFields ({ formTypeField, jidField });
			form.toXml (writer);
		}

		if (Count_ > 0 || !ItemId_.isEmpty ())
		{
			QXmppResultSetQuery q;
			if (Count_ > 0)
				q.setMax (Count_);
			if (!ItemId_.isEmpty ())
			{
				switch (Dir_)
				{
				case Direction::After:
					q.setAfter (ItemId_);
					break;
				case Direction::Before:
					q.setBefore (ItemId_);
					break;
				default:
					break;
				}
			}
			else if (!ItemId_.isNull () && Dir_ == Direction::Before)
				q.setBefore ("");
			q.toXml (writer);
		}
	}
}
}
}
