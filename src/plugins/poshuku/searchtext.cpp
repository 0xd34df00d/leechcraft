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

#include "searchtext.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			SearchText::SearchText (const QString& text, QWidget *parent)
			: QDialog (parent)
			, Text_ (text)
			{
				Ui_.setupUi (this);
				Ui_.Label_->setText (tr ("Search %1 with:").arg (text));

				QStringList categories = Core::Instance ().GetProxy ()->GetSearchCategories ();
				Q_FOREACH (QString cat, categories)
					new QTreeWidgetItem (Ui_.Tree_, QStringList (cat));

				on_MarkAll__released ();

				connect (this,
						SIGNAL (accepted ()),
						this,
						SLOT (doSearch ()));
			}

			void SearchText::doSearch ()
			{
				QStringList selected;

				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
					if (Ui_.Tree_->topLevelItem (i)->checkState (0) == Qt::Checked)
						selected << Ui_.Tree_->topLevelItem (i)->text (0);

				if (!selected.size ())
					return;

				QString query = Text_;

				if (selected.size () == 1)
					query = QString ("%1 ca:%2").arg (Text_).arg (selected.at (0));
				else
				{
					query += " (";
					query += QString ("ca:%1").arg (selected.at (0));
					for (int i = 1; i < selected.size (); ++i)
						query += QString (" OR ca:%1").arg (selected.at (i));
					query += ")";
				}

				Core::Instance ().GetProxy ()->OpenSummary (query);
			}

			void SearchText::on_MarkAll__released ()
			{
				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
					Ui_.Tree_->topLevelItem (i)->setCheckState (0, Qt::Checked);
			}

			void SearchText::on_UnmarkAll__released ()
			{
				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
					Ui_.Tree_->topLevelItem (i)->setCheckState (0, Qt::Unchecked);
			}
		};
	};
};

