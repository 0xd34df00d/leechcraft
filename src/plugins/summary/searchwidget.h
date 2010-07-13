/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_SUMMARY_SEARCHWIDGET_H
#define PLUGINS_SUMMARY_SEARCHWIDGET_H
#include <QDockWidget>
#include "ui_searchwidget.h"

namespace LeechCraft
{
	namespace Util
	{
		class CategorySelector;
	};

	namespace Plugins
	{
		namespace Summary
		{
			class SearchWidget : public QDockWidget
			{
				Q_OBJECT

				Ui::SearchWidget Ui_;
				Util::CategorySelector *CategorySelector_;
			public:
				SearchWidget (QWidget* = 0);

				QLineEdit* GetFilterLine () const;
				QComboBox* GetFilterType () const;
				bool IsOr () const;

				QStringList GetCategories () const;
				void SetPossibleCategories (const QStringList&);
				void SelectCategories (const QStringList& subset);
			signals:
				void paramsChanged ();
			};
		};
	};
};

#endif

