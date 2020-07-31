/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historymessage.h"
#include <QtDebug>
#include <interfaces/azoth/imucentry.h>

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	HistoryMessage::HistoryMessage (IMessage::Direction dir,
			QObject *otherPart,
			Type type,
			const QString& variant,
			const QString& body,
			const QDateTime& dt,
			const QString& richBody,
			EscapePolicy policy)
	: Direction_ (dir)
	, OtherPart_ (otherPart)
	, Type_ (type)
	, Variant_ (variant)
	, Body_ (body)
	, DateTime_ (dt)
	, RichBody_ (richBody)
	, EscPolicy_ (policy)
	{
	}

	QObject* HistoryMessage::GetQObject ()
	{
		return this;
	}

	void HistoryMessage::Send ()
	{
		qWarning () << Q_FUNC_INFO
				<< "unable to send history message";
	}

	void HistoryMessage::Store ()
	{
		qWarning () << Q_FUNC_INFO
				<< "unable to store history message";
	}

	IMessage::Direction HistoryMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::Type HistoryMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType HistoryMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* HistoryMessage::OtherPart () const
	{
		return OtherPart_;
	}

	QString HistoryMessage::GetOtherVariant () const
	{
		return Variant_;
	}

	QString HistoryMessage::GetBody () const
	{
		return Body_;
	}

	void HistoryMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QDateTime HistoryMessage::GetDateTime () const
	{
		return DateTime_;
	}

	void HistoryMessage::SetDateTime (const QDateTime& dt)
	{
		DateTime_ = dt;
	}

	QString HistoryMessage::GetRichBody () const
	{
		return RichBody_;
	}

	void HistoryMessage::SetRichBody (const QString& body)
	{
		RichBody_ = body;
	}

	IMessage::EscapePolicy HistoryMessage::GetEscapePolicy () const
	{
		return EscPolicy_;
	}
}
}
}
