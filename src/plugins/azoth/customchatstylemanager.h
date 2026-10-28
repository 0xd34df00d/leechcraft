/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <utility>
#include <QObject>

namespace LC
{
namespace Azoth
{
	class IAccount;
	class ICLEntry;

	class CustomChatStyleManager : public QObject
	{
		Q_OBJECT
	public:
		CustomChatStyleManager (QObject* = 0);

		std::pair<QString, QString> GetForEntry (ICLEntry*) const;

		std::pair<QString, QString> GetStyleForAccount (IAccount*) const;
		std::pair<QString, QString> GetMUCStyleForAccount (IAccount*) const;

		enum class Settable
		{
			ChatStyle,
			ChatVariant,
			MUCStyle,
			MUCVariant
		};
		void Set (IAccount*, Settable, const QString&);
	private:
		std::pair<QString, QString> GetProps (const QString&, IAccount*) const;
	signals:
		void accountStyleChanged (IAccount*);
	};
}
}
