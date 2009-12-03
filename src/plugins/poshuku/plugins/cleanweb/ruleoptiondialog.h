/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_RULEOPTIONDIALOG_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_RULEOPTIONDIALOG_H
#include <QDialog>
#include "ui_ruleoptiondialog.h"
#include "filter.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace CleanWeb
				{
					class RuleOptionDialog : public QDialog
					{
						Q_OBJECT

						Ui::RuleOptionDialog Ui_;
					public:
						RuleOptionDialog (QWidget* = 0);

						QString GetString () const;
						void SetString (const QString&);
						bool IsException () const;
						void SetException (bool);
						FilterOption::MatchType GetType () const;
						void SetType (FilterOption::MatchType);
						Qt::CaseSensitivity GetCase () const;
						void SetCase (Qt::CaseSensitivity);
						QStringList GetDomains () const;
						void SetDomains (const QStringList&);
						QStringList GetNotDomains () const;
						void SetNotDomains (const QStringList&);
					private:
						void Add (QComboBox*);
						void Modify (QComboBox*);
						void Remove (QComboBox*);
					private slots:
						void on_AddEnabled__released ();
						void on_ModifyEnabled__released ();
						void on_RemoveEnabled__released ();
						void on_AddDisabled__released ();
						void on_ModifyDisabled__released ();
						void on_RemoveDisabled__released ();
						void invalidateButtons ();
					};
				};
			};
		};
	};
};

#endif

