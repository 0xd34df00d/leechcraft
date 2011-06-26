/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "editkeyworddialog.h"


namespace LeechCraft
{
namespace Poshuku
{
namespace Keywords
{ 

	EditKeywordDialog::EditKeywordDialog (const QString& url, 
		const QString& keyword, QWidget *parent)
	: QDialog (parent)
	, Url_ (url)
	, Keyword_ (keyword)
	{
		Ui_.setupUi (this);
		Ui_.Url_->setText (Url_);
		Ui_.Keyword_->setText (Keyword_);
	}

	QString EditKeywordDialog::GetUrl () const
	{
		return Url_;
	}

	QString EditKeywordDialog::GetKeyword () const
	{
		return Keyword_;
	}

	void EditKeywordDialog::on_Buttons__accepted()
	{
		Url_ = Ui_.Url_->text ();
		Keyword_ = Ui_.Keyword_->text ();
	}
}
}
}

