/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QStringList>
#include <QXmppDataForm.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class AdHocCommand
	{
		QString Name_;
		QString Node_;
	public:
		AdHocCommand (const QString& name, const QString& node);

		QString GetName () const;
		void SetName (const QString&);

		QString GetNode () const;
		void SetNode (const QString&);
	};

	class AdHocNote
	{
	public:
		enum class Severity
		{
			Info,
			Warn,
			Error
		};
	private:
		Severity Severity_;
		QString Text_;
	public:
		AdHocNote (const QDomElement&);
		AdHocNote (const QString& severity, const QString& text);
		AdHocNote (Severity severity, const QString& text);

		Severity GetSeverity () const;
		const QString& GetText () const;
	};

	class AdHocResult
	{
		QString Node_;
		QString SessionID_;

		QXmppDataForm Form_;

		QStringList Actions_;

		QList<AdHocNote> Notes_;
	public:
		QString GetNode () const;
		void SetNode (const QString&);

		QString GetSessionID () const;
		void SetSessionID (const QString&);

		QXmppDataForm GetDataForm () const;
		void SetDataForm (const QXmppDataForm&);

		QStringList GetActions () const;
		void SetActions (const QStringList&);

		const QList<AdHocNote>& GetNotes () const;
		void AddNote (const AdHocNote&);
	};
}
}
}
