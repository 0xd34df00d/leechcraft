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

#include "flashonclickwhitelist.h"
#include <QCoreApplication>
#include <QStandardItemModel>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QtDebug>

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
					FlashOnClickWhitelist::FlashOnClickWhitelist (QWidget *parent)
					: QWidget (parent)
					, Model_ (new QStandardItemModel (this))
					{
						Model_->setHorizontalHeaderLabels (QStringList (tr ("Whitelist")));

						QSettings settings (QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_CleanWeb");
						settings.beginGroup ("FlashOnClick");
						int size = settings.beginReadArray ("Whitelist");
						for (int i = 0; i < size; ++i)
						{
							settings.setArrayIndex (i);
							Model_->appendRow (new QStandardItem (settings
										.value ("Exception").toString ()));
						}
						settings.endArray ();
						settings.endGroup ();

						Ui_.setupUi (this);
						Ui_.WhitelistTree_->setModel (Model_);
					}

					QStringList FlashOnClickWhitelist::GetWhitelist () const
					{
						QStringList result;
						for (int i = 0, rowCount = Model_->rowCount (); i < rowCount; ++i)
							result << Model_->item (i)->text ();
						return result;
					}

					bool FlashOnClickWhitelist::Matches (const QString& str) const
					{
						Q_FOREACH (QString white, GetWhitelist ())
						{
							if (str.indexOf (white) >= 0 ||
									str.indexOf (QRegExp (white)) >= 0)
								return true;
						}
						return false;
					}

					void FlashOnClickWhitelist::Add (const QString& str)
					{
						AddImpl (str);
					}

					void FlashOnClickWhitelist::on_Add__released ()
					{
						AddImpl ();
					}

					void FlashOnClickWhitelist::on_Modify__released ()
					{
						QModelIndex index = Ui_.WhitelistTree_->currentIndex ();
						if (!index.isValid ())
							return;

						QString str = Model_->itemFromIndex (index)->text ();
						AddImpl (str, index);
					}

					void FlashOnClickWhitelist::on_Remove__released ()
					{
						QModelIndex index = Ui_.WhitelistTree_->currentIndex ();
						if (!index.isValid ())
							return;

						qDeleteAll (Model_->takeRow (index.row ()));
						SaveSettings ();
					}

					void FlashOnClickWhitelist::AddImpl (QString str, const QModelIndex& old)
					{
						bool ok = false;
						str = QInputDialog::getText (this,
								tr ("Add URL to whitelist"),
								tr ("Please enter the URL to add to the FlashOnClick's whitelist"),
								QLineEdit::Normal,
								str,
								&ok);
						if (str.isEmpty () ||
								!ok)
							return;

						if (old.isValid ())
							qDeleteAll (Model_->takeRow (old.row ()));

						if (Matches (str))
						{
							QMessageBox::warning (this,
									tr ("LeechCraft"),
									tr ("This URL is already matched by another whitelist entry."));
							return;
						}

						Model_->appendRow (new QStandardItem (str));
						SaveSettings ();
					}

					void FlashOnClickWhitelist::SaveSettings ()
					{
						QSettings settings (QCoreApplication::organizationName (),
								QCoreApplication::applicationName () + "_CleanWeb");
						settings.beginGroup ("FlashOnClick");
						settings.beginWriteArray ("Whitelist");
						settings.remove ("");
						for (int i = 0, rowCount = Model_->rowCount (); i < rowCount; ++i)
						{
							settings.setArrayIndex (i);
							settings.setValue ("Exception", Model_->item (i)->text ());
						}
						settings.endArray ();
						settings.endGroup ();
					}
				};
			};
		};
	};
};

