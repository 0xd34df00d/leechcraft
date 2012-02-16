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

#include "cmwrapper.h"
#include <QtDebug>
#include <ConnectionManager>
#include <PendingReady>
#include "protowrapper.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	CMWrapper::CMWrapper (const QString& cmName, QObject *parent)
	: QObject (parent)
	, CM_ (Tp::ConnectionManager::create (cmName))
	{
		connect (CM_->becomeReady (),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleCMReady (Tp::PendingOperation*)));
	}

	QList<QObject*> CMWrapper::GetProtocols () const
	{
		QList<QObject*> result;
		Q_FOREACH (auto pw, ProtoWrappers_)
			result << pw;
		return result;
	}

	void CMWrapper::handleCMReady (Tp::PendingOperation *op)
	{
		if (op->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< CM_->name ()
					<< op->errorName ()
					<< op->errorMessage ();
			return;
		}

		qDebug () << Q_FUNC_INFO << CM_->name ();
		QList<QObject*> newProtoWrappers;
		Q_FOREACH (const QString& proto, CM_->supportedProtocols ())
		{
			qDebug () << "has protocol" << proto;
			if (proto == "jabber" || proto == "irc")
				continue;

			auto pw = new ProtoWrapper (CM_, proto, this);
			ProtoWrappers_ << pw;
			newProtoWrappers << pw;
		}

		emit gotProtoWrappers (newProtoWrappers);
	}
}
}
}
