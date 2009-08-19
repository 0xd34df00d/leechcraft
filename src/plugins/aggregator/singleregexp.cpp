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

#include <QtDebug>
#include "singleregexp.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			namespace
			{
				inline bool IsRegexpValid (const QString& rx)
				{
					QString str = rx;
					if (rx.startsWith ("\\link"))
						str = rx.right (rx.size () - 5);
					return QRegExp (str).isValid () && !QRegExp (str).isEmpty ();
				}
			};
			
			SingleRegexp::SingleRegexp (const QString& title,
					const QString& body,
					bool modifier,
					QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				connect (Ui_.TitleEdit_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (lineEdited (const QString&)));
				connect (Ui_.BodyEdit_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (lineEdited (const QString&)));
			
				Ui_.TitleEdit_->setText (title);
				Ui_.BodyEdit_->setText (body);
			
				if (modifier)
					Ui_.TitleEdit_->setEnabled (false);
			
				lineEdited (title, Ui_.TitleEdit_);
				lineEdited (body, Ui_.BodyEdit_);
			}
			
			QString SingleRegexp::GetTitle () const
			{
				return Ui_.TitleEdit_->text ();
			}
			
			QString SingleRegexp::GetBody () const
			{
				return Ui_.BodyEdit_->text ();
			}
			
			void SingleRegexp::lineEdited (const QString& newText, QWidget *setter)
			{
				if (IsRegexpValid (newText))
					(setter ? setter : qobject_cast<QWidget*> (sender ()))->
						setStyleSheet ("background-color: rgba(0, 255, 0, 50);");
				else
					(setter ? setter : qobject_cast<QWidget*> (sender ()))->
						setStyleSheet ("background-color: rgba(255, 0, 0, 50);");
			}
		};
	};
};

