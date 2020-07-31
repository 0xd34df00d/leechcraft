/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC::Snails
{
	struct AccountConfig
	{
		QString AccName_;
		QString UserName_;
		QString UserEmail_;

		QString Login_;
		bool UseSASL_ = false;
		bool SASLRequired_ = false;

		enum class SecurityType
		{
			TLS,
			SSL,
			No
		};
		SecurityType InSecurity_ = SecurityType::TLS;
		bool InSecurityRequired_ = false;
		SecurityType OutSecurity_ = SecurityType::SSL;
		bool OutSecurityRequired_ = false;

		bool SMTPNeedsAuth_ = true;

		QString InHost_;
		int InPort_;

		QString OutHost_;
		int OutPort_;

		QString OutLogin_;

		enum class OutType
		{
			SMTP,
			Sendmail
		};
		OutType OutType_;

		int KeepAliveInterval_ = 90 * 1000;
		bool LogToFile_ = true;

		enum class DeleteBehaviour
		{
			Default,
			Expunge,
			MoveToTrash
		};

		DeleteBehaviour DeleteBehaviour_ = DeleteBehaviour::Default;
	};

	QDataStream& operator<< (QDataStream&, const AccountConfig&);
	QDataStream& operator>> (QDataStream&, AccountConfig&);
}
