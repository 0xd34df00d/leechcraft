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

#pragma once

#include <QXmppIq.h>
#include <QDateTime>

namespace LeechCraft
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
