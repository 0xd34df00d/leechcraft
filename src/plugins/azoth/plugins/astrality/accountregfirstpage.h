/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_accountregfirstpage.h"
#include "accountwrapper.h"

namespace Tp
{
	class ProtocolInfo;
}

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	class AccountRegFirstPage : public QWidget
	{
		Q_OBJECT

		Ui::AccountRegFirstPage Ui_;
	public:
		AccountRegFirstPage (const Tp::ProtocolInfo&, bool, QWidget* = 0);

		void SetParams (const QVariantMap&);

		QString GetAccountID () const;
		QString GetPassword () const;
		QString GetServer () const;
		int GetPort () const;
		bool ShouldRequireEncryption () const;

		void SetSettings (const AccountWrapper::Settings&);
		void Augment (AccountWrapper::Settings&) const;
	};
}
}
}
