/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "vacuumimportpage.h"
#include <QStandardItemModel>

namespace LeechCraft
{
namespace NewLife
{
namespace Importers
{
	VacuumImportPage::VacuumImportPage (QWidget *parent)
	: QWizardPage (parent)
	, AccountsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
	}

	bool VacuumImportPage::isComplete () const
	{
		return true;
	}

	int VacuumImportPage::nextId () const
	{
		return -1;
	}

	void VacuumImportPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()),
				Qt::UniqueConnection);
		connect (this,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				wizard (),
				SIGNAL (gotEntity (LeechCraft::Entity)));

		AccountsModel_->clear ();

		QStringList labels;
		labels << tr ("Account name")
				<< tr ("JID")
				<< tr ("Import account settings")
				<< tr ("Import history");
		AccountsModel_->setHorizontalHeaderLabels (labels);

		//FindAccounts ();
	}
}
}
}
