/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "ircparser.h"
#include <boost/bind.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <QTextCodec>
#include "ircaccount.h"
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	using namespace boost::spirit::classic;

	IrcParser::IrcParser (IrcServerHandler *sh)
	: ISH_ (sh)
	, ServerOptions_ (sh->GetServerOptions ())
	{
	}

	void IrcParser::AuthCommand ()
	{
		QString pass = ServerOptions_.ServerPassword_;
		if (!pass.isEmpty ())
		{
			QString passCmd = QString ("PASS " + pass + "\r\n");
			ISH_->SendCommand (passCmd);
		}

		UserCommand ();
		NickCommand ();
	}

	void IrcParser::UserCommand ()
	{
		QString userCmd = QString ("USER " +
				ISH_->GetAccount ()->GetUserName () + " 0 * :" +
				ISH_->GetAccount ()->GetRealName () + "\r\n");
		ISH_->SendCommand (userCmd);
	}

	void IrcParser::NickCommand ()
	{
		QString nickCmd = QString ("NICK " + 
				ISH_->GetAccount ()->GetOurNick () + "\r\n");
		ISH_->SendCommand (nickCmd);
	}
};
};
};
