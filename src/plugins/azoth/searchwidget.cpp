/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "searchwidget.h"
#include <QTimer>
#include <util/sll/qobjectrefcast.h>
#include <util/sys/resourceloader.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/ihavesearch.h"
#include "interfaces/azoth/iextselfinfoaccount.h"
#include "core.h"
#include "resourcesmanager.h"
#include "avatarsmanager.h"

namespace LC
{
namespace Azoth
{
	QObject* SearchWidget::S_ParentMultiTabs_ = 0;

	void SearchWidget::SetParentMultiTabs (QObject *parent)
	{
		S_ParentMultiTabs_ = parent;
	}

	namespace
	{
		Util::ContextTask<void> SetAvatar (AvatarsManager& am, QComboBox& box, int idx, IExtSelfInfoAccount *self)
		{
			co_await Util::AddContext { Util::CoroContext::Of (box) };
			std::optional<QImage> avatar;
			const auto accountIcon = self ? self->GetAccountIcon () : QIcon {};
			if (auto selfEntry = self ? self->GetSelfContact () : nullptr)
			{
				const auto& id = qobject_ref_cast<ICLEntry> (selfEntry).GetGlobalStrongestId ();
				const auto dim = box.iconSize ().width ();
				avatar = co_await am.GetAvatarImage (id, IAvatarsManager::Scaled { dim });
			}

			QIcon icon { avatar ? QPixmap::fromImage (*avatar) : QPixmap {} };
			if (icon.isNull ())
				icon = accountIcon;

			if (!icon.isNull ())
				box.setItemIcon (idx, icon);
		}
	}

	SearchWidget::SearchWidget (AvatarsManager *am, QWidget *parent)
	: QWidget { parent }
	, AvatarsManager_ { am }
	{
		Ui_.setupUi (this);

		connect (Ui_.Server_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (search ()));

		const QIcon defaultAvatarIcon
		{
			QPixmap::fromImage (ResourcesManager::Instance ()
					.GetDefaultAvatar (Ui_.AccountBox_->iconSize ().width ()))
		};

		for (const auto acc : Core::Instance ().GetAccounts ())
		{
			const auto accObj = acc->GetQObject ();
			const auto search = qobject_cast<IHaveSearch*> (accObj);
			if (!search)
				continue;

			const auto self = qobject_cast<IExtSelfInfoAccount*> (accObj);

			const auto idx = Ui_.AccountBox_->count ();

			Ui_.AccountBox_->blockSignals (true);
			Ui_.AccountBox_->addItem (defaultAvatarIcon,
					acc->GetAccountName (),
					QVariant::fromValue<QObject*> (accObj));
			Ui_.AccountBox_->blockSignals (false);

			SetAvatar (*AvatarsManager_, *Ui_.AccountBox_, idx, self);
		}

		Ui_.AccountBox_->blockSignals (true);
		Ui_.AccountBox_->blockSignals (false);
	}

	TabClassInfo SearchWidget::GetTabClassInfo () const
	{
		TabClassInfo searchTab =
		{
			"Search",
			tr ("Search"),
			tr ("A search tab allows one to search within IM services"),
			QIcon ("lcicons:/plugins/azoth/resources/images/searchtab.svg"),
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
		emit removeTab ();
		deleteLater ();
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
		if (!search)
		{
			qWarning () << Q_FUNC_INFO
					<< "no current search object";
			return;
		}

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

		if (Ui_.ResultsTree_->selectionModel ())
			Ui_.ResultsTree_->selectionModel ()->deleteLater ();
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
