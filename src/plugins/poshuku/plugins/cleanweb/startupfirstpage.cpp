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

#include "startupfirstpage.h"
#include <typeinfo>
#include <QLineEdit>
#include <QTextCodec>
#include <QComboBox>
#include <QMessageBox>
#include <QRadioButton>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"

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
					StartupFirstPage::StartupFirstPage (QWidget *parent)
					: QWizardPage (parent)
					{
						Ui_.setupUi (this);
					}

					void StartupFirstPage::initializePage ()
					{
						connect (wizard (),
								SIGNAL (accepted ()),
								this,
								SLOT (handleAccepted ()));
					}

					namespace
					{
						QList<QUrl> GetChildUrls (QWidget *w)
						{
							QList<QUrl> result;
							Q_FOREACH (QCheckBox *cb, w->findChildren<QCheckBox*> ())
								if (cb->isChecked ())
									result << cb->property ("ListURL").value<QUrl> ();

							Q_FOREACH (QRadioButton *but, w->findChildren<QRadioButton*> ())
								if (but->isChecked ())
									result << but->property ("ListURL").value<QUrl> ();
							return result;
						}
					};

					void StartupFirstPage::handleAccepted ()
					{
						QList<QUrl> urlsToAdd;

						Q_FOREACH (QGroupBox *box, findChildren<QGroupBox*> ())
							if (box->isChecked ())
							{
								urlsToAdd << box->property ("ListURL").value<QUrl> ();
								urlsToAdd << GetChildUrls (box);
							}

						urlsToAdd << GetChildUrls (Ui_.VariousListsPage_);

						Q_FOREACH (QUrl url, urlsToAdd)
							Core::Instance ().Add (url);
					}
				};
			};
		};
	};
};

