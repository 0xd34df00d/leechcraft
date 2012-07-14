/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_MATCHCONFIGDIALOG_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_MATCHCONFIGDIALOG_H
#include <memory>
#include <QDialog>
#include "ui_matchconfigdialog.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class TypedMatcherBase;
	typedef std::shared_ptr<TypedMatcherBase> TypedMatcherBase_ptr;

	class FieldMatch;

	class MatchConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::MatchConfigDialog Ui_;

		QSet<QString> Types_;
		TypedMatcherBase_ptr CurrentMatcher_;
	public:
		MatchConfigDialog (const QStringList&, QWidget* = 0);

		FieldMatch GetFieldMatch () const;
	private slots:
		void on_SourcePlugin__activated (int);
		void on_FieldName__activated (int);
	};
}
}

#endif
