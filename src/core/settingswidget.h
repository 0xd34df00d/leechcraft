/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QWidget>
#include "ui_settingswidget.h"

class IHaveSettings;

namespace LC
{
	class SettingsWidget : public QWidget
	{
		Q_OBJECT

		Ui::SettingsWidget Ui_;

		QHash<QTreeWidgetItem*, QPair<IHaveSettings*, int>> Item2Page_;
	public:
		using MatchesGetter_t = std::function<QHash<IHaveSettings*, QList<int>> ()>;
	private:
		const MatchesGetter_t MatchesGetter_;
	public:
		SettingsWidget (QObject *settable,
				const QObjectList& subplugins,
				const MatchesGetter_t& matchesGetter,
				QWidget *parent = nullptr);
		~SettingsWidget ();

		void Accept ();
		void Reject ();

		void UpdateSearchHighlights ();
	private:
		void FillPages (QObject*, bool);
		QSet<IHaveSettings*> GetUniqueIHS () const;
	private slots:
		void on_Cats__currentItemChanged (QTreeWidgetItem*);
	};
}
