/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QStringList>
#include "entrybase.h"
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;

	class ServerParticipantEntry : public EntryBase
	{
		Q_OBJECT

		QString NickName_;
		QString ServerKey_;
		QStringList Channels_;
		bool PrivateChat_;

		IrcAccount *Account_;
		QHash<QString, ChannelRole> Channel2Role_;
	public:
		ServerParticipantEntry (const QString&,
				const QString&, IrcAccount*);

		QObject* GetParentAccount () const ;
		QObject* GetParentCLEntry () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		QStringList GetChannels () const;
		void SetPrivateChat (bool);
		bool IsPrivateChat () const;

		ChannelRole GetRole (const QString&) const;
		void SetRole (const QString&, ChannelRole);
	public slots:
		void closePrivateChat (bool);
	};

	typedef boost::shared_ptr<ServerParticipantEntry>
			ServerParticipantEntry_ptr;
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H
