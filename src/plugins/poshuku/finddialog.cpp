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

#include "finddialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			FindDialog::FindDialog (QWidget *parent)
			: Notification (parent)
			{
				Ui_.setupUi (this);
			}
			
			FindDialog::~FindDialog ()
			{
			}
			
			void FindDialog::SetSuccessful (bool success)
			{
				QString ss = QString ("QLineEdit {"
						"background-color:rgb(");
				if (success)
					ss.append ("0,255");
				else
					ss.append ("255,0");
				ss.append (",0) }");
				Ui_.Pattern_->setStyleSheet (ss);
			}

			void FindDialog::Focus ()
			{
				Ui_.Pattern_->setFocus ();
			}
			
			void FindDialog::on_Pattern__textChanged (const QString& newText)
			{
				Ui_.FindButton_->setEnabled (!newText.isEmpty ());
			}
			
			void FindDialog::on_FindButton__released ()
			{
				QWebPage::FindFlags flags;
				if (Ui_.SearchBackwards_->checkState () == Qt::Checked)
					flags |= QWebPage::FindBackward;
				if (Ui_.MatchCase_->checkState () == Qt::Checked)
					flags |= QWebPage::FindCaseSensitively;
				if (Ui_.WrapAround_->checkState () == Qt::Checked)
					flags |= QWebPage::FindWrapsAroundDocument;
			
				emit next (Ui_.Pattern_->text (), flags);
			}
			
			void FindDialog::reject ()
			{
				hide ();
			}
		};
	};
};

