/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountconfig.h"
#include <QDataStream>

namespace LC::Snails
{
	QDataStream& operator<< (QDataStream& out, const AccountConfig& cfg)
	{
		out << static_cast<quint8> (1);
		out << cfg.AccName_
			<< cfg.Login_
			<< cfg.UseSASL_
			<< cfg.SASLRequired_
			<< static_cast<qint8> (cfg.InSecurity_)
			<< cfg.InSecurityRequired_
			<< static_cast<qint8> (cfg.OutSecurity_)
			<< cfg.OutSecurityRequired_
			<< cfg.SMTPNeedsAuth_
			<< cfg.InHost_
			<< cfg.InPort_
			<< cfg.OutHost_
			<< cfg.OutPort_
			<< cfg.OutLogin_
			<< static_cast<quint8> (cfg.OutType_)
			<< cfg.UserName_
			<< cfg.UserEmail_
			<< cfg.KeepAliveInterval_
			<< cfg.LogToFile_
			<< static_cast<quint8> (cfg.DeleteBehaviour_);
		return out;
	}

	QDataStream& operator>> (QDataStream& in, AccountConfig& cfg)
	{
		quint8 version = 0;
		in >> version;

		if (version != 1)
			throw std::runtime_error { "Unknown version " + std::to_string (version) };

		quint8 outType = 0;
		qint8 inSec = 0;
		qint8 outSec = 0;

		in >> cfg.AccName_
			>> cfg.Login_
			>> cfg.UseSASL_
			>> cfg.SASLRequired_
			>> inSec
			>> cfg.InSecurityRequired_
			>> outSec
			>> cfg.OutSecurityRequired_;

		cfg.InSecurity_ = static_cast<AccountConfig::SecurityType> (inSec);
		cfg.OutSecurity_ = static_cast<AccountConfig::SecurityType> (outSec);

		in >> cfg.SMTPNeedsAuth_
			>> cfg.InHost_
			>> cfg.InPort_
			>> cfg.OutHost_
			>> cfg.OutPort_
			>> cfg.OutLogin_
			>> outType;

		cfg.OutType_ = static_cast<AccountConfig::OutType> (outType);

		in >> cfg.UserName_
			>> cfg.UserEmail_
			>> cfg.KeepAliveInterval_
			>> cfg.LogToFile_;

		quint8 deleteBehaviour = 0;
		in >> deleteBehaviour;
		cfg.DeleteBehaviour_ = static_cast<AccountConfig::DeleteBehaviour> (deleteBehaviour);

		return in;
	}
}
