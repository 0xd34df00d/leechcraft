/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaType>
#include <QVariant>
#include <QStringList>
#include <QIcon>

namespace LC
{
namespace Blogique
{
	/** @brief Interface representing an account's profile.
		*
		* This interface represents an account's profile.
		**/
	class IProfile
	{
	public:
		virtual ~IProfile () {}

		virtual QWidget* GetProfileWidget () = 0;

		virtual QList<QPair<QIcon, QString>> GetPostingTargets () const = 0;
	protected:
		virtual void profileUpdated () = 0;

	};
}
}

Q_DECLARE_INTERFACE (LC::Blogique::IProfile,
		"org.Deviant.LeechCraft.Blogique.IProfile/1.0")
