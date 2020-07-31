/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QObject>
#include <interfaces/azoth/icanhavesslerrors.h>

namespace LC
{
namespace Azoth
{
	class SslErrorsHandler : public QObject
	{
		Q_OBJECT

	public:
		struct AccountRegistration {};
		struct Account
		{
			QByteArray ID_;
			QString Name_;
		};

		using Context_t = std::variant<AccountRegistration, Account>;
	private:
		const Context_t Context_;
		ICanHaveSslErrors * const ICHSE_;
	public:
		SslErrorsHandler (const Context_t&, ICanHaveSslErrors*);
	private slots:
		void sslErrors (const QList<QSslError>&,
				const ICanHaveSslErrors::ISslErrorsReaction_ptr&);
	};
}
}
