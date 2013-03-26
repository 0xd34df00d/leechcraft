/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "findnotification.h"
#include "ui_findnotification.h"

namespace LeechCraft
{
namespace Util
{
	FindNotification::FindNotification (QWidget *parent)
	: Util::PageNotification (parent)
	, Ui_ (new Ui::FindNotification)
	{
		Ui_->setupUi (this);

		setFocusProxy (Ui_->Pattern_);
	}

	FindNotification::~FindNotification ()
	{
		delete Ui_;
	}

	void FindNotification::SetText (const QString& text)
	{
		Ui_->Pattern_->setText (text);
	}

	QString FindNotification::GetText () const
	{
		return Ui_->Pattern_->text ();
	}

	void FindNotification::SetSuccessful (bool success)
	{
		QString ss = QString ("QLineEdit {"
				"background-color:rgb(");
		if (success)
			ss.append ("0,255");
		else
			ss.append ("255,0");
		ss.append (",0) }");
		Ui_->Pattern_->setStyleSheet (ss);
	}

	void FindNotification::Focus ()
	{
		Ui_->Pattern_->setFocus ();
	}

	auto FindNotification::GetFlags () const -> FindFlags
	{
		FindFlags flags;
		if (Ui_->MatchCase_->checkState () == Qt::Checked)
			flags |= FindCaseSensitively;
		if (Ui_->WrapAround_->checkState () == Qt::Checked)
			flags |= FindWrapsAround;
		return flags;
	}

	void FindNotification::findNext ()
	{
		const auto& text = GetText ();
		if (text.isEmpty ())
			return;

		handleNext (text, GetFlags ());
	}

	void FindNotification::findPrevious ()
	{
		const auto& text = GetText ();
		if (text.isEmpty ())
			return;

		handleNext (text, GetFlags () | FindBackwards);
	}

	void FindNotification::on_Pattern__textChanged (const QString& newText)
	{
		Ui_->FindButton_->setEnabled (!newText.isEmpty ());
	}

	void FindNotification::on_FindButton__released ()
	{
		auto flags = GetFlags ();
		if (Ui_->SearchBackwards_->checkState () == Qt::Checked)
			flags |= FindBackwards;

		handleNext (Ui_->Pattern_->text (), flags);
	}

	void FindNotification::reject ()
	{
		hide ();
	}
}
}
