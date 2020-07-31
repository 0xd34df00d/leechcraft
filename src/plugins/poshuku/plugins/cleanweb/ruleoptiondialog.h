/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_ruleoptiondialog.h"
#include "filter.h"

namespace LC
{
namespace Poshuku
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
}
}
}
