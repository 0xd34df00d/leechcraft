/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "adhoccommand.h"
#include <QtDebug>
#include <QDomElement>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	AdHocCommand::AdHocCommand (const QString& name, const QString& node)
	: Name_ { name }
	, Node_ { node }
	{
	}

	QString AdHocCommand::GetName () const
	{
		return Name_;
	}

	void AdHocCommand::SetName (const QString& name)
	{
		Name_ = name;
	}

	QString AdHocCommand::GetNode () const
	{
		return Node_;
	}

	void AdHocCommand::SetNode (const QString& node)
	{
		Node_ = node;
	}

	namespace
	{
		AdHocNote::Severity Type2Severity (const QString& severity)
		{
			if (severity == "info")
				return AdHocNote::Severity::Info;
			else if (severity == "warn")
				return AdHocNote::Severity::Warn;
			else if (severity == "error")
				return AdHocNote::Severity::Error;

			qWarning () << Q_FUNC_INFO
					<< "unknown severity level"
					<< severity;
			return AdHocNote::Severity::Info;
		}
	}

	AdHocNote::AdHocNote (const QDomElement& elem)
	: AdHocNote { elem.attribute ("type"), elem.text () }
	{
	}

	AdHocNote::AdHocNote (const QString& severity, const QString& text)
	: AdHocNote { Type2Severity (severity), text }
	{
	}

	AdHocNote::AdHocNote (Severity severity, const QString& text)
	: Severity_ { severity }
	, Text_ { text }
	{
	}

	AdHocNote::Severity AdHocNote::GetSeverity () const
	{
		return Severity_;
	}

	const QString& AdHocNote::GetText () const
	{
		return Text_;
	}

	QString AdHocResult::GetNode () const
	{
		return Node_;
	}

	void AdHocResult::SetNode (const QString& node)
	{
		Node_ = node;
	}

	QString AdHocResult::GetSessionID () const
	{
		return SessionID_;
	}

	void AdHocResult::SetSessionID (const QString& sid)
	{
		SessionID_ = sid;
	}

	QXmppDataForm AdHocResult::GetDataForm () const
	{
		return Form_;
	}

	void AdHocResult::SetDataForm (const QXmppDataForm& form)
	{
		Form_ = form;
	}

	QStringList AdHocResult::GetActions () const
	{
		return Actions_;
	}

	void AdHocResult::SetActions (const QStringList& actions)
	{
		Actions_ = actions;
	}

	const QList<AdHocNote>& AdHocResult::GetNotes () const
	{
		return Notes_;
	}

	void AdHocResult::AddNote (const AdHocNote& note)
	{
		Notes_ << note;
	}
}
}
}
