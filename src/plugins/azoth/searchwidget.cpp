/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "searchwidget.h"
#include <QTimer>
#include <util/resourceloader.h>
#include "interfaces/iaccount.h"
#include "interfaces/ihavesearch.h"
#include "interfaces/iextselfinfoaccount.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	QObject* SearchWidget::S_ParentMultiTabs_ = 0;

	void SearchWidget::SetParentMultiTabs (QObject *parent)
	{
		S_ParentMultiTabs_ = parent;
	}

	SearchWidget::SearchWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.Server_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (search ()));

		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
		{
			QObject *accObj = acc->GetObject ();
			IHaveSearch *search = qobject_cast<IHaveSearch*> (accObj);
			if (!search)
				continue;

			IExtSelfInfoAccount *self = qobject_cast<IExtSelfInfoAccount*> (accObj);
			ICLEntry *selfEntry = self ?
					qobject_cast<ICLEntry*> (self->GetSelfContact ()) :
					0;
			QIcon icon;
			if (selfEntry)
				icon = QPixmap::fromImage (selfEntry->GetAvatar ());
			if (icon.isNull () && self)
				icon = self->GetAccountIcon ();
			if (icon.isNull ())
			{
				const QString& name = XmlSettingsManager::Instance ()
						.property ("SystemIcons").toString () + "/default_avatar";
				Util::ResourceLoader *rl = Core::Instance ()
						.GetResourceLoader (Core::RLTSystemIconLoader);
				icon = QIcon (rl->GetIconPath (name));
			}

			Ui_.AccountBox_->blockSignals (true);
			Ui_.AccountBox_->addItem (icon, acc->GetAccountName (),
					QVariant::fromValue<QObject*> (accObj));
			Ui_.AccountBox_->blockSignals (false);
		}
	}

	TabClassInfo SearchWidget::GetTabClassInfo () const
	{
		TabClassInfo searchTab =
		{
			"Search",
			tr ("Search"),
			tr ("A search tab allows to search within IM services"),
			QIcon (),
			55,
			TFOpenableByRequest
		};
		return searchTab;
	}

	QObject* SearchWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	void SearchWidget::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* SearchWidget::GetToolBar () const
	{
		return 0;
	}

	IHaveSearch* SearchWidget::GetCurrentSearch () const
	{
		const int idx = Ui_.AccountBox_->currentIndex ();
		QObject *accObj = Ui_.AccountBox_->itemData (idx).value<QObject*> ();
		return qobject_cast<IHaveSearch*> (accObj);
	}

	void SearchWidget::search ()
	{
		IHaveSearch *search = GetCurrentSearch ();

		QObject *sessObj = search->CreateSearchSession ();
		ISearchSession *sess = qobject_cast<ISearchSession*> (sessObj);
		CurrentSess_.reset (sess);

		if (!sess)
		{
			qWarning () << Q_FUNC_INFO
					<< sessObj
					<< "doesn't implement ISearchSession";
			return;
		}

		sess->RestartSearch (Ui_.Server_->text ());

		Ui_.ResultsTree_->setModel (sess->GetRepresentationModel ());
	}

	void SearchWidget::on_AccountBox__activated (int)
	{
		IHaveSearch *s = GetCurrentSearch ();
		Ui_.Server_->setText (s->GetDefaultSearchServer ());

		QTimer::singleShot (0,
				this,
				SLOT (search ()));
	}
}
}
