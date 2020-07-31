/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QFlags>
#include <QMetaType>

namespace LC
{
namespace Azoth
{
	class IRegManagedAccount
	{
	public:
		virtual ~IRegManagedAccount () {}

		enum class Feature
		{
			UpdatePass,
			DeregisterAcc
		};

		virtual bool SupportsFeature (Feature) const = 0;

		virtual void UpdateServerPassword (const QString& newPass) = 0;

		virtual void DeregisterAccount () = 0;
	protected:
		virtual void serverPasswordUpdated (const QString&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IRegManagedAccount,
		"org.Deviant.LeechCraft.Azoth.IRegManagedAccount/1.0")
