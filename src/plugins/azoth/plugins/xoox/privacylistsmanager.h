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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_PRIVACYLISTSMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_PRIVACYLISTSMANAGER_H
#include <QXmppClientExtension.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class PrivacyListItem
	{
	public:
		enum Type
		{
			TNone,
			TJid,
			TGroup,
			TSubscription
		};
		
		enum Action
		{
			AAllow,
			ADeny
		};
		
		enum StanzaType
		{
			STNone = 0x00,
			STMessage = 0x01,
			STPresenceIn = 0x02,
			STPresenceOut = 0x04,
			STIq = 0x08,
			STAll = 0x0f
		};
		
		Q_DECLARE_FLAGS (StanzaTypes, StanzaType);
	private:
		QString Value_;
		Type Type_;
		Action Action_;
		StanzaTypes Stanzas_;
	public:
		PrivacyListItem (const QString& = QString (), Type = TNone, Action = ADeny);
		
		void Parse (const QDomElement&);
		QXmppElement ToXML () const;
		
		Type GetType () const;
		void SetType (Type);
		
		Action GetAction () const;
		void SetAction (Action);
		
		QString GetValue () const;
		void SetValue (const QString&);
		
		StanzaTypes GetStanzaTypes () const;
		void SetStanzaTypes (StanzaTypes);
	};

	class PrivacyList
	{
		QString Name_;
		QList<PrivacyListItem> Items_;
	public:
		PrivacyList (const QString& = QString ());
		
		void Parse (const QDomElement&);
		QXmppElement ToXML () const;
		
		QString GetName () const;
		void SetName (const QString&);
		
		QList<PrivacyListItem> GetItems () const;
		void SetItems (const QList<PrivacyListItem>&);
	};

	class PrivacyListsManager : public QXmppClientExtension
	{
		Q_OBJECT
		
		enum QueryType
		{
			QTQueryLists,
			QTGetList
		};
		QMap<QString, QueryType> ID2Type_;
	public:
		enum ListType
		{
			LTActive,
			LTDefault
		};

		void QueryLists ();
		void QueryList (const QString&);
		void ActivateList (const QString&, ListType = LTActive);
		void SetList (const PrivacyList&);

		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement&);
	private:
		void HandleListQueryResult (const QDomElement&);
		void HandleList (const QDomElement&);
	signals:
		void gotLists (const QStringList&, const QString&, const QString&);
		void gotList (const PrivacyList&);
	};
}
}
}

#endif
