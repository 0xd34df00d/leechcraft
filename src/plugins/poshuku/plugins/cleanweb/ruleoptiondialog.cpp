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

#include "ruleoptiondialog.h"
#include <QButtonGroup>
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
					RuleOptionDialog::RuleOptionDialog (QWidget *parent)
					: QDialog (parent)
					{
						Ui_.setupUi (this);

						QButtonGroup *policyGroup = new QButtonGroup (this);
						policyGroup->setExclusive (true);
						policyGroup->addButton (Ui_.Allow_);
						policyGroup->addButton (Ui_.Block_);

						QButtonGroup *typeGroup = new QButtonGroup (this);
						typeGroup->setExclusive (true);
						typeGroup->addButton (Ui_.Wildcard_);
						typeGroup->addButton (Ui_.Regexp_);

						connect (Ui_.EnabledDomains_,
								SIGNAL (currentIndexChanged (int)),
								this,
								SLOT (invalidateButtons ()));
						connect (Ui_.DisabledDomains_,
								SIGNAL (currentIndexChanged (int)),
								this,
								SLOT (invalidateButtons ()));

						connect (Ui_.String_,
								SIGNAL (textChanged (const QString&)),
								this,
								SLOT (invalidateButtons ()));
					}

					QString RuleOptionDialog::GetString () const
					{
						return Ui_.String_->text ();
					}

					void RuleOptionDialog::SetString (const QString& str)
					{
						Ui_.String_->setText (str);
					}

					bool RuleOptionDialog::IsException () const
					{
						return Ui_.Allow_->isChecked ();
					}

					void RuleOptionDialog::SetException (bool ex)
					{
						Ui_.Allow_->setChecked (ex);
					}

					FilterOption::MatchType RuleOptionDialog::GetType () const
					{
						return Ui_.Wildcard_->isChecked () ?
							FilterOption::MTWildcard :
							FilterOption::MTRegexp;
					}

					void RuleOptionDialog::SetType (FilterOption::MatchType mt)
					{
						Ui_.Wildcard_->setChecked (mt == FilterOption::MTWildcard);
					}

					Qt::CaseSensitivity RuleOptionDialog::GetCase () const
					{
						return Ui_.CaseSensitive_->checkState () == Qt::Checked ?
							Qt::CaseSensitive :
							Qt::CaseInsensitive;
					}

					void RuleOptionDialog::SetCase (Qt::CaseSensitivity cs)
					{
						Ui_.CaseSensitive_->setCheckState (cs == Qt::CaseSensitive ?
								Qt::Checked :
								Qt::Unchecked);
					}

					QStringList RuleOptionDialog::GetDomains () const
					{
						QStringList result;
						for (int i = 0; i < Ui_.EnabledDomains_->count (); ++i)
							result << Ui_.EnabledDomains_->itemText (i);
						return result;
					}

					void RuleOptionDialog::SetDomains (const QStringList& domains)
					{
						Ui_.EnabledDomains_->clear ();
						Ui_.EnabledDomains_->addItems (domains);
					}

					QStringList RuleOptionDialog::GetNotDomains () const
					{
						QStringList result;
						for (int i = 0; i < Ui_.DisabledDomains_->count (); ++i)
							result << Ui_.DisabledDomains_->itemText (i);
						return result;
					}

					void RuleOptionDialog::SetNotDomains (const QStringList& domains)
					{
						Ui_.DisabledDomains_->clear ();
						Ui_.DisabledDomains_->addItems (domains);
					}

					void RuleOptionDialog::Add (QComboBox *box)
					{
						bool ok;
						QString domain = QInputDialog::getText (this,
								"LeechCraft",
								tr ("Enter domain"),
								QLineEdit::Normal,
								QString (),
								&ok);

						if (!ok ||
								domain.isEmpty ())
							return;

						box->addItem (domain);
					}

					void RuleOptionDialog::Modify (QComboBox *box)
					{
						int idx = box->currentIndex ();
						if (idx < 0)
						{
							qWarning () << Q_FUNC_INFO
								<< box
								<< "current index < 0";
							return;
						}

						bool ok;
						QString domain = QInputDialog::getText (this,
								"LeechCraft",
								tr ("Enter domain"),
								QLineEdit::Normal,
								box->itemText (idx),
								&ok);

						if (!ok ||
								domain.isEmpty ())
							return;

						box->setItemText (idx, domain);
					}

					void RuleOptionDialog::Remove (QComboBox *box)
					{
						int idx = box->currentIndex ();
						if (idx < 0)
						{
							qWarning () << Q_FUNC_INFO
								<< box
								<< "current index < 0";
							return;
						}

						if (QMessageBox::question (this,
									"LeechCraft",
									tr ("Are you sure you want to remove %1?")
										.arg (box->itemText (idx)),
									QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
							return;

						box->removeItem (idx);
					}

					void RuleOptionDialog::on_AddEnabled__released ()
					{
						Add (Ui_.EnabledDomains_);
					}

					void RuleOptionDialog::on_ModifyEnabled__released ()
					{
						Modify (Ui_.EnabledDomains_);
					}

					void RuleOptionDialog::on_RemoveEnabled__released ()
					{
						Remove (Ui_.EnabledDomains_);
					}

					void RuleOptionDialog::on_AddDisabled__released ()
					{
						Add (Ui_.DisabledDomains_);
					}

					void RuleOptionDialog::on_ModifyDisabled__released ()
					{
						Modify (Ui_.DisabledDomains_);
					}

					void RuleOptionDialog::on_RemoveDisabled__released ()
					{
						Remove (Ui_.DisabledDomains_);
					}

					void RuleOptionDialog::invalidateButtons ()
					{
						bool hasEnabledDomains = Ui_.EnabledDomains_->currentIndex () >= 0;
						Ui_.ModifyEnabled_->setEnabled (hasEnabledDomains);
						Ui_.RemoveEnabled_->setEnabled (hasEnabledDomains);

						bool hasDisabledDomains = Ui_.DisabledDomains_->currentIndex () >= 0;
						Ui_.ModifyDisabled_->setEnabled (hasDisabledDomains);
						Ui_.RemoveDisabled_->setEnabled (hasDisabledDomains);

						QPushButton *ok = Ui_.ButtonBox_->button (QDialogButtonBox::Ok);
						if (!ok)
							qWarning () << Q_FUNC_INFO
								<< "OK button is null :(";
						else
						{
							ok->setEnabled (Ui_.String_->text ().size ());
						}
					}
				};
			};
		};
	};
};

