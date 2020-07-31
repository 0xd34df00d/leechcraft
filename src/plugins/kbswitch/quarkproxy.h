/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace KBSwitch
{
	class QuarkProxy : public QObject
	{
		Q_OBJECT

		Q_PROPERTY (QString currentLangCode READ GetCurrentLangCode NOTIFY currentLangCodeChanged)

		QString CurrentLangCode_;
	public:
		QuarkProxy (QObject* = 0);

		QString GetCurrentLangCode () const;
	public slots:
		void setNextLanguage ();
		void contextMenuRequested ();
	private slots:
		void handleGroupSelectAction ();
		void handleGroupChanged (int);
	signals:
		void currentLangCodeChanged ();
	};
}
}
