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

#include <QMessageBox>
#include <QHeaderView>
#include "singleregexp.h"
#include "regexpmatchermanager.h"
#include "regexpmatcherui.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			RegexpMatcherUi::RegexpMatcherUi (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				Ui_.Regexps_->setModel (&RegexpMatcherManager::Instance ());
			}
			
			RegexpMatcherUi::~RegexpMatcherUi ()
			{
				RegexpMatcherManager::Instance ().Release ();
			}
			
			void RegexpMatcherUi::on_AddRegexpButton__released ()
			{
				bool success = false;
				QString title, body;
				do
				{
					success = true;
					SingleRegexp srx (title, body, false, this);
					if (srx.exec () == QDialog::Rejected)
						return;
			
					title = srx.GetTitle ();
					body = srx.GetBody ();
			
					try
					{
						RegexpMatcherManager::Instance ().Add (title, body);
					}
					catch (const RegexpMatcherManager::AlreadyExists&)
					{
						QMessageBox::critical (this,
								tr ("LeechCraft"),
								tr ("This title "
									"matcher regexp already exists. Specify another "
									"one or modify existing title matcher regexp's "
									"body extractor."));
						success = false;
					}
					catch (const RegexpMatcherManager::Malformed&)
					{
						QMessageBox::critical (this,
								tr ("LeechCraft"),
								tr ("Either title"
									" matcher or body extractor is malformed."));
						success = false;
					}
				}
				while (!success);
			}
			
			void RegexpMatcherUi::on_ModifyRegexpButton__released ()
			{
				QModelIndex index = Ui_.Regexps_->selectionModel ()->currentIndex ();
				if (!index.isValid ())
					return;
			
				bool success = false;
				RegexpMatcherManager::titlebody_t pair = RegexpMatcherManager::Instance ().GetTitleBody (index);
				QString title = pair.first,
						body = pair.second;
				do
				{
					success = true;
					SingleRegexp srx (title, body, true, this);
					if (srx.exec () == QDialog::Rejected)
						return;
			
					body = srx.GetBody ();
			
					try
					{
						RegexpMatcherManager::Instance ().Modify (title, body);
					}
					catch (const RegexpMatcherManager::AlreadyExists&)
					{
						QMessageBox::critical (this,
								tr ("LeechCraft"),
								tr ("This title "
									"matcher regexp already exists. Specify another "
									"one or modify existing title matcher regexp's "
									"body extractor."));
						success = false;
					}
					catch (const RegexpMatcherManager::Malformed&)
					{
						QMessageBox::critical (this,
								tr ("LeechCraft"),
								tr ("Either title"
									" matcher or body extractor is malformed."));
						success = false;
					}
				}
				while (!success);
			}
			
			void RegexpMatcherUi::on_RemoveRegexpButton__released ()
			{
				QModelIndex index = Ui_.Regexps_->selectionModel ()->currentIndex ();
				if (!index.isValid ())
					return;
			
				RegexpMatcherManager::Instance ().Remove (index);
			}
		};
	};
};

