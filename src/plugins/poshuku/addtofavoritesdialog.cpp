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

#include "addtofavoritesdialog.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			using LeechCraft::Util::TagsCompleter;
			using LeechCraft::Util::TagsCompletionModel;
			
			AddToFavoritesDialog::AddToFavoritesDialog (const QString& title,
					const QString& url,
					QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				Ui_.URLLabel_->setText (url);
				Ui_.TitleEdit_->setText (title);
				Ui_.TagsEdit_->setText (tr ("untagged"));
			
				TagsCompleter_.reset (new TagsCompleter (Ui_.TagsEdit_));
				Ui_.TagsEdit_->AddSelector ();
			}
			
			AddToFavoritesDialog::~AddToFavoritesDialog ()
			{
			}
			
			QString AddToFavoritesDialog::GetTitle () const
			{
				return Ui_.TitleEdit_->text ();
			}
			
			QStringList AddToFavoritesDialog::GetTags () const
			{
				return Core::Instance ().GetProxy ()->
					GetTagsManager ()->Split (Ui_.TagsEdit_->text ());
			}
		};
	};
};

