/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_acceptlangwidget.h"
#include "localeentry.h"

namespace LC
{
namespace Intermutko
{
	class LocalesModel;

	class AcceptLangWidget : public QWidget
	{
		Q_OBJECT

		Ui::AcceptLangWidget Ui_;
		LocalesModel * const Model_;

		QList<LocaleEntry> Locales_;
		QString LocaleStr_;
	public:
		AcceptLangWidget (QWidget* = 0);

		const QString& GetLocaleString () const;
	private:
		void AddLocale (const LocaleEntry&);

		void WriteSettings ();
		void LoadSettings ();

		void RebuildLocaleStr ();
	public slots:
		void accept ();
		void reject ();
	private slots:
		void on_Add__released ();
		void on_Remove__released ();
		void on_MoveUp__released ();
		void on_MoveDown__released ();
		void on_Language__currentIndexChanged (int);
	};
}
}
