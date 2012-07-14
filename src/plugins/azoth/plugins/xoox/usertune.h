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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERTUNE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERTUNE_H
#include <QString>
#include <QUrl>
#include "pepeventbase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class UserTune : public PEPEventBase
	{
		QString Artist_;
		QString Source_;
		QString Title_;
		QString Track_;
		QUrl URI_;
		int Length_;
		int Rating_;
	public:
		static QString GetNodeString ();
		
		QXmppElement ToXML () const;
		void Parse (const QDomElement&);
		QString Node () const;
		
		PEPEventBase* Clone () const;
		
		QString GetArtist () const;
		void SetArtist (const QString&);

		QString GetSource () const;
		void SetSource (const QString&);

		QString GetTitle () const;
		void SetTitle (const QString&);

		QString GetTrack () const;
		void SetTrack (const QString&);

		QUrl GetURI () const;
		void SetURI (const QUrl&);

		int GetLength () const;
		void SetLength (int);

		int GetRating () const;
		void SetRating (int);
		
		bool IsNull () const;
	};
}
}
}

#endif
