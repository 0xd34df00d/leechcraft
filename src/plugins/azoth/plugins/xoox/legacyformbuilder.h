/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/function.hpp>
#include <QObject>
#include <QHash>
#include <QCoreApplication>
#include <QXmppElement.h>

class QWidget;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class LegacyFormBuilder : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::Xoox::LegacyFormBuilder)

		QWidget *Widget_;

		using ElementActor_t = std::function<void (QWidget*, const QXmppElement&)>;
		QHash<QString, ElementActor_t> Tag2Actor_;
	public:
		LegacyFormBuilder ();

		void Clear ();

		QWidget* CreateForm (const QXmppElement&, QWidget* = 0);
		QList<QXmppElement> GetFilledChildren () const;

		QString GetUsername () const;
		QString GetPassword () const;
	};
}
}
}
