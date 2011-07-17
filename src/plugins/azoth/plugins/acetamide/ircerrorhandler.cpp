/*
    LeechCraft - modular cross-platform feature rich internet client.
    Copyright (C) 2010-2011 Oleg Linkin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "ircerrorhandler.h"
#include <QTextCodec>
#include <util/util.h>
#include <util/notificationactionhandler.h>
#include "core.h"
#include "ircserverhandler.h"


namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcErrorHandler::IrcErrorHandler (IrcServerHandler *ish)
	: QObject (ish)
	, ISH_ (ish)
	{
		InitErrors ();
	}

	void IrcErrorHandler::HandleError (int id, 
			const QList<std::string>& params, const QString& message)
	{
		if (!IsError (id))
			return;

		QString msg, paramsMessage = QString ();
		QTextCodec *codec = QTextCodec::codecForName (ISH_->GetServerOptions ()
				.ServerEncoding_.toUtf8 ());
		msg = codec->toUnicode (message.toAscii ());
		
		if (!params.count () > 1)
			Q_FOREACH (const std::string& str, params.mid (1))
				paramsMessage += QString::fromUtf8 (str.c_str ()) + " ";

		Entity e = Util::MakeNotification ("Azoth", 
				(paramsMessage.isEmpty ()) ? msg : (paramsMessage + ": " + msg), 
				PWarning_);
		Core::Instance ().SendEntity (e);
	}

	bool IrcErrorHandler::IsError (int id)
	{
		return ErrorKeys_.contains (id);
	}

	void IrcErrorHandler::InitErrors ()
	{
		ErrorKeys_ << 401;
		ErrorKeys_ << 402;
		ErrorKeys_ << 403;
		ErrorKeys_ << 404;
		ErrorKeys_ << 405;
		ErrorKeys_ << 406;
		ErrorKeys_ << 407;
		ErrorKeys_ << 408;
		ErrorKeys_ << 409;
		ErrorKeys_ << 411;
		ErrorKeys_ << 412;
		ErrorKeys_ << 413;
		ErrorKeys_ << 414;
		ErrorKeys_ << 415;
		ErrorKeys_ << 421;
		ErrorKeys_ << 422;
		ErrorKeys_ << 424;
		ErrorKeys_ << 431;
		ErrorKeys_ << 432;
		ErrorKeys_ << 433;
		ErrorKeys_ << 436;
		ErrorKeys_ << 437;
		ErrorKeys_ << 441;
		ErrorKeys_ << 442;
		ErrorKeys_ << 443;
		ErrorKeys_ << 444;
		ErrorKeys_ << 445;
		ErrorKeys_ << 446;
		ErrorKeys_ << 451;
		ErrorKeys_ << 461;
		ErrorKeys_ << 462;
		ErrorKeys_ << 463;
		ErrorKeys_ << 464;
		ErrorKeys_ << 465;
		ErrorKeys_ << 466;
		ErrorKeys_ << 467;
		ErrorKeys_ << 471;
		ErrorKeys_ << 472;
		ErrorKeys_ << 473;
		ErrorKeys_ << 474;
		ErrorKeys_ << 475;
		ErrorKeys_ << 476;
		ErrorKeys_ << 477;
		ErrorKeys_ << 478;
		ErrorKeys_ << 481;
		ErrorKeys_ << 482;
		ErrorKeys_ << 483;
		ErrorKeys_ << 484;
		ErrorKeys_ << 485;
		ErrorKeys_ << 491;
		ErrorKeys_ << 501;
		ErrorKeys_ << 502;
	}
};
};
};