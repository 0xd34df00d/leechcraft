/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkmessage.h"
#include "entrybase.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	VkMessage::VkMessage (bool isOurs, Direction dir, Type type, EntryBase *parentEntry, EntryBase *other)
	: QObject (parentEntry)
	, OtherPart_ (other)
	, ParentCLEntry_ (parentEntry)
	, Type_ (type)
	, Dir_ (dir)
	, IsOurs_ (isOurs)
	{
	}

	QObject* VkMessage::GetQObject ()
	{
		return this;
	}

	void VkMessage::Send ()
	{
		ParentCLEntry_->Send (this);
		Store ();
	}

	void VkMessage::Store ()
	{
		ParentCLEntry_->Store (this);
	}

	qulonglong VkMessage::GetID () const
	{
		return ID_;
	}

	void VkMessage::SetID (qulonglong id)
	{
		ID_ = id;
		emit messageDelivered ();
	}

	bool VkMessage::IsRead () const
	{
		return IsRead_;
	}

	void VkMessage::SetRead ()
	{
		IsRead_ = true;
	}

	QString VkMessage::GetRawBody () const
	{
		return Body_;
	}

	IMessage::Direction VkMessage::GetDirection () const
	{
		return Dir_;
	}

	IMessage::Type VkMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::SubType VkMessage::GetMessageSubType () const
	{
		return SubType::Other;
	}

	QObject* VkMessage::OtherPart () const
	{
		return OtherPart_ ? OtherPart_ : ParentCLEntry_;
	}

	QObject* VkMessage::ParentCLEntry() const
	{
		return ParentCLEntry_;
	}

	QString VkMessage::GetOtherVariant () const
	{
		return "";
	}

	QString VkMessage::GetBody () const
	{
		auto result = Body_;
		if (IsOurs_)
			result.replace ('<', "&lt;");
		return result;
	}

	void VkMessage::SetBody (const QString& body)
	{
		Body_ = body;
	}

	IMessage::EscapePolicy VkMessage::GetEscapePolicy () const
	{
		return EscapePolicy::NoEscape;
	}

	QDateTime VkMessage::GetDateTime () const
	{
		return TS_;
	}

	void VkMessage::SetDateTime (const QDateTime& timestamp)
	{
		TS_ = timestamp;
	}

	bool VkMessage::IsDelivered () const
	{
		return ID_ != static_cast<qulonglong> (-1);
	}

	QString VkMessage::GetRichBody () const
	{
		return RichBody_;
	}

	void VkMessage::SetRichBody (const QString& body)
	{
		RichBody_ = body;
	}
}
}
}
