/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCJOINGROUPCHAT_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCJOINGROUPCHAT_H

#include <QWidget>
#include <interfaces/imucjoinwidget.h>
#include "ui_ircjoingroupchat.h"
#include "core.h"
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class IrcAccount;
	
	class IrcJoinGroupChat : public QWidget
						   , public IMUCJoinWidget
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMUCJoinWidget)
		
		Ui::IrcJoinGroupChat Ui_;
		IrcAccount *SelectedAccount_;
	public:
		IrcJoinGroupChat (QWidget* = 0);
		
		void AccountSelected (QObject*);
		void Join (QObject*);
		void Cancel ();
		QVariantList GetBookmarkedMUCs () const;
		void SetBookmarkedMUCs (QObject*, const QVariantList&);
		void SetIdentifyingData (const QVariantMap&);
		QVariantMap GetIdentifyingData () const;
		
		QString GetServer () const;
		int GetPort () const;
		QString GetChannel () const;
		QString GetNickname () const;
		QString GetEncoding () const;
		bool GetSSL () const;

		ServerOptions GetServerOptions () const;
		ChannelOptions GetChannelOptions () const;
	signals:
		void validityChanged (bool);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCJOINGROUPCHAT_H
