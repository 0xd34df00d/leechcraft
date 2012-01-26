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

#include "firstpage.h"
#include <QVariant>
#include "abstractimporter.h"

namespace LeechCraft
{
namespace NewLife
{
	FirstPage::FirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
	}

	int FirstPage::nextId () const
	{
		return StartPages_ [GetImporter ()];
	}

	void FirstPage::SetupImporter (AbstractImporter *ai)
	{
		const auto& names = ai->GetNames ();
		const auto& icons = ai->GetIcons ();
		for (int i = 0; i < std::min (names.size (), icons.size ()); ++i)
			Ui_.SourceApplication_->addItem (icons.at (i),
					names.at (i),
					QVariant::fromValue<QObject*> (ai));

		QList<QWizardPage*> pages = ai->GetWizardPages ();
		if (pages.size ())
		{
			QWizardPage *first = pages.takeFirst ();
			StartPages_ [ai] = wizard ()->addPage (first);
			Q_FOREACH (QWizardPage *page, pages)
				wizard ()->addPage (page);
		}
	}

	AbstractImporter* FirstPage::GetImporter () const
	{
		int currentIndex = Ui_.SourceApplication_->currentIndex ();
		if (currentIndex == -1)
			return 0;

		QObject *importerObject = Ui_.SourceApplication_->
			itemData (currentIndex).value<QObject*> ();
		return qobject_cast<AbstractImporter*> (importerObject);
	}

	QString FirstPage::GetSelectedName () const
	{
		return Ui_.SourceApplication_->currentText ();
	}
}
}
