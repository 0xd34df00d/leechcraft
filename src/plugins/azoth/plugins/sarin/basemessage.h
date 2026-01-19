/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/imessage.h>

namespace LC::Azoth::Sarin
{
	class BaseMessage
		: public QObject
		, public IMessage
	{
		Q_INTERFACES (LC::Azoth::IMessage)
	protected:
		QString Body_;
		QDateTime TS_ = QDateTime::currentDateTime ();

		const Direction Dir_;
		const Type Type_;
		const SubType SubType_;
	public:
		explicit BaseMessage (Direction, Type, SubType, const QString& body, QObject* = nullptr);

		QObject* GetQObject () override;

		Direction GetDirection () const override;
		Type GetMessageType () const override;
		SubType GetMessageSubType () const override;

		QString GetBody () const override;
		void SetBody (const QString& body) override;
		QDateTime GetDateTime () const override;
		void SetDateTime (const QDateTime& timestamp) override;
	};
}
