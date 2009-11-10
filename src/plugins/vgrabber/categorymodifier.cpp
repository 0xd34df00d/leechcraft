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

#include "categorymodifier.h"
#include <QPushButton>
#include <plugininterface/tagscompleter.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			CategoryModifier::CategoryModifier (const QString& text, QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);

				Completer_ = new Util::TagsCompleter (Ui_.Line_);
				Ui_.Line_->AddSelector ();
				Ui_.Line_->setText (text);
			}

			QString CategoryModifier::GetText () const
			{
				return Ui_.Line_->text ();
			}

			void CategoryModifier::on_Line__textChanged (const QString& text)
			{
				Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (text.size ());
			}
		};
	};
};

