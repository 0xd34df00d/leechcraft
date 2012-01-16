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

#pragma once

#include <QWizardPage>
#include "ui_psiplusimportpage.h"

class QStandardItemModel;
class QStandardItem;
class QDomElement;

namespace LeechCraft
{
struct Entity;

namespace NewLife
{
namespace Importers
{
	class PsiPlusImportPage : public QWizardPage
	{
		Q_OBJECT

		Ui::PsiPlusImportPage Ui_;

		QStandardItemModel *AccountsModel_;

		enum Roles
		{
			AccountData = Qt::UserRole + 1
		};

		enum Column
		{
			AccountName,
			JID,
			ImportAcc,
			ImportHist
		};
	public:
		PsiPlusImportPage (QWidget* = 0);

		bool isComplete () const;
		int nextId () const;
		void initializePage ();
	private:
		void FindAccounts ();
		void ScanProfile (const QString&, const QString&);
		void ScanAccount (QStandardItem*, const QDomElement&);

		void SendImportAcc (QStandardItem*);
		void SendImportHist (QStandardItem*);
	private slots:
		void handleAccepted ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}
