/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppIq.h>
#include <QDateTime>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class XMPPAnnotationsIq : public QXmppIq
	{
	public:
		class NoteItem
		{
			QString Jid_;
			QString Note_;
			QDateTime CDate_;
			QDateTime MDate_;
		public:
			NoteItem ();
			NoteItem (const QString&, const QString&);

			QString GetJid () const;
			void SetJid (const QString&);

			QString GetNote () const;
			void SetNote (const QString&);

			QDateTime GetCDate () const;
			void SetCDate (const QDateTime&);

			QDateTime GetMDate () const;
			void SetMDate (const QDateTime&);
		};
	private:
		QList<NoteItem> Items_;
	public:
		XMPPAnnotationsIq ();

		QList<NoteItem> GetItems () const;
		void SetItems (const QList<NoteItem>&);
	protected:
		void parseElementFromChild (const QDomElement&);
		void toXmlElementFromChild (QXmlStreamWriter*) const;
	};
}
}
}
