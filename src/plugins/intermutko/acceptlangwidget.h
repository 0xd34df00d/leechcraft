/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ui_acceptlangwidget.h"
#include "localeentry.h"

namespace LC::Intermutko
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
		explicit AcceptLangWidget (QWidget* = nullptr);

		const QString& GetLocaleString () const;
	private:
		void WriteSettings ();
		void LoadSettings ();

		void RebuildLocaleStr ();

		void RunAddLocaleDialog ();
	public slots:
		void accept ();
		void reject ();
	};
}
