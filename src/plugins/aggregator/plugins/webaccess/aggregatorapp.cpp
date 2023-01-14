/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "aggregatorapp.h"
#include <QObject>
#include <QThread>
#include <QtDebug>
#include <Wt/WText.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WBoxLayout.h>
#include <Wt/WCheckBox.h>
#include <Wt/WTreeView.h>
#include <Wt/WTableView.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WOverlayLoadingIndicator.h>
#include <Wt/WPanel.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WCssTheme.h>
#include <interfaces/aggregator/iproxyobject.h>
#include <interfaces/aggregator/channel.h>
#include <interfaces/aggregator/iitemsmodel.h>
#include <util/aggregator/itemsmodeldecorator.h>
#include "readchannelsfilter.h"
#include "util.h"
#include "q2wproxymodel.h"
#include "readitemsfilter.h"

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	namespace
	{
		class WittyThread : public QThread
		{
			Wt::WApplication * const App_;
		public:
			WittyThread (Wt::WApplication *app)
			: App_ { app }
			{
				setObjectName ("Aggregator WebAccess (Wt Thread)");
			}
		protected:
			void run ()
			{
				App_->attachThread (true);
				QThread::run ();
				App_->attachThread (false);
			}
		};
	}

	AggregatorApp::AggregatorApp (IProxyObject *ap, ICoreProxy_ptr cp, const Wt::WEnvironment& environment)
	: WApplication { environment }
	, AP_ { ap }
	, CP_ { cp }
	, ObjsThread_ { new WittyThread (this) }
	, ChannelsModel_ { std::make_shared<Q2WProxyModel> (*AP_->GetChannelsModel (), this) }
	, ChannelsFilter_ { std::make_shared<ReadChannelsFilter> () }
	, SourceItemModel_ { AP_->CreateItemsModel () }
	, ItemsModel_ { std::make_shared<Q2WProxyModel> (SourceItemModel_->GetQModel (), this) }
	, ItemsFilter_ { std::make_shared<ReadItemsFilter> () }
	{
		ChannelsModel_->SetRoleMappings ({
				{ ChannelRole::UnreadCount, Aggregator::ChannelRoles::UnreadCount },
				{ ChannelRole::CID, Aggregator::ChannelRoles::ChannelID }
			});

		ItemsModel_->SetRoleMappings ({
				{ ItemRole::IID, Aggregator::IItemsModel::ItemRole::ItemId },
				{ ItemRole::IsRead, Aggregator::IItemsModel::ItemRole::IsRead }
			});
		ItemsModel_->AddDataMorphism ([] (const QModelIndex& idx, Wt::ItemDataRole role) -> Wt::cpp17::any
			{
				if (role != Wt::ItemDataRole::StyleClass)
					return {};

				if (!idx.data (Aggregator::IItemsModel::ItemRole::IsRead).toBool ())
					return Wt::WString ("unreadItem");

				return {};
			});

		auto initThread = [this] (QObject *obj)
		{
			obj->moveToThread (ObjsThread_);
			QObject::connect (ObjsThread_,
					SIGNAL (finished ()),
					obj,
					SLOT (deleteLater ()));
		};
		initThread (ChannelsModel_.get ());
		initThread (ItemsModel_.get ());
		SourceItemModel_->GetQModel ().moveToThread (ObjsThread_);

		ObjsThread_->start ();

		ChannelsFilter_->setSourceModel (ChannelsModel_);
		ItemsFilter_->setSourceModel (ItemsModel_);

		setTitle ("Aggregator WebAccess");
		setLoadingIndicator (std::make_unique<Wt::WOverlayLoadingIndicator> ());

		SetupUI ();

		enableUpdates (true);
	}

	AggregatorApp::~AggregatorApp ()
	{
		ObjsThread_->quit ();
		ObjsThread_->wait (1000);
		if (!ObjsThread_->isFinished ())
		{
			qWarning () << Q_FUNC_INFO
					<< "objects thread hasn't finished yet, terminating...";
			ObjsThread_->terminate ();
		}

		delete ObjsThread_;
	}

	void AggregatorApp::HandleChannelClicked (const Wt::WModelIndex& idx, const Wt::WMouseEvent&)
	{
		ItemView_->setText ({});

		const auto cid = Wt::cpp17::any_cast<IDType_t> (idx.data (ChannelRole::CID));

		ItemsFilter_->ClearCurrentItem ();
		ItemsModelDecorator { *SourceItemModel_ }.Reset (cid);
	}

	void AggregatorApp::HandleItemClicked (const Wt::WModelIndex& idx, const Wt::WMouseEvent& event)
	{
		if (!idx.isValid ())
			return;

		const auto itemId = Wt::cpp17::any_cast<IDType_t> (idx.data (ItemRole::IID));
		const auto& item = AP_->GetItem (itemId);
		if (!item)
			return;

		ItemsFilter_->SetCurrentItem (itemId);

		switch (event.button ())
		{
		case Wt::MouseButton::Left:
			ShowItem (*item);
			break;
		case Wt::MouseButton::Right:
			ShowItemMenu (*item, event);
			break;
		default:
			break;
		}
	}

	void AggregatorApp::ShowItem (const Item& item)
	{
		AP_->SetItemRead (item.ItemID_, true);

		auto text = Wt::WString ("<div><a href='{1}' target='_blank'>{2}</a><br />{3}<br /><hr/>{4}</div>")
				.arg (ToW (item.Link_))
				.arg (ToW (item.Title_))
				.arg (ToW (item.PubDate_.toString ()))
				.arg (ToW (item.Description_));
		ItemView_->setText (text);
	}

	void AggregatorApp::ShowItemMenu (const Item& item, const Wt::WMouseEvent& event)
	{
		Wt::WPopupMenu menu;
		const auto itemId = item.ItemID_;
		if (item.Unread_)
			menu.addItem (ToW (tr ("Mark as read")))->
					triggered ().connect ([this, itemId] { AP_->SetItemRead (itemId, true); });
		else
			menu.addItem (ToW (tr ("Mark as unread")))->
					triggered ().connect ([this, itemId] { AP_->SetItemRead (itemId, false); });
		menu.exec (event);
	}

	void AggregatorApp::SetupUI ()
	{
		setTheme (std::make_shared<Wt::WCssTheme> ("polished"));
		setLocale ({ QLocale {}.name ().toUtf8 ().constData () });

		styleSheet ().addRule (".unreadItem", "font-weight: bold;");

		auto rootLay = root ()->setLayout (std::make_unique<Wt::WBoxLayout> (Wt::LayoutDirection::LeftToRight));

		auto leftPaneLay = rootLay->addLayout (std::make_unique<Wt::WBoxLayout> (Wt::LayoutDirection::TopToBottom), 2);

		auto showReadChannels = leftPaneLay->addWidget (std::make_unique<Wt::WCheckBox> (ToW (tr ("Include read channels"))));
		showReadChannels->setToolTip (ToW (tr ("Also display channels that have no unread items.")));
		showReadChannels->setChecked (false);
		showReadChannels->checked ().connect ([this] { ChannelsFilter_->SetHideRead (false); });
		showReadChannels->unChecked ().connect ([this] { ChannelsFilter_->SetHideRead (true); });

		auto channelsTree = leftPaneLay->addWidget (std::make_unique<Wt::WTreeView> (), 1, Wt::AlignmentFlag::Top);
		channelsTree->setModel (ChannelsFilter_);
		channelsTree->setSelectionMode (Wt::SelectionMode::Single);
		channelsTree->clicked ().connect (this, &AggregatorApp::HandleChannelClicked);
		channelsTree->setAlternatingRowColors (true);

		auto rightPaneLay = rootLay->addLayout (std::make_unique<Wt::WBoxLayout> (Wt::LayoutDirection::TopToBottom), 7);

		auto showReadItems = rightPaneLay->addWidget (std::make_unique<Wt::WCheckBox> (ToW (tr ("Show read items"))));
		showReadItems->setChecked (false);
		showReadItems->checked ().connect ([this] { ItemsFilter_->SetHideRead (false); });
		showReadItems->unChecked ().connect ([this] { ItemsFilter_->SetHideRead (true); });

		ItemsTable_ = rightPaneLay->addWidget (std::make_unique<Wt::WTableView> (), 2, Wt::AlignmentFlag::Justify);
		ItemsTable_->setModel (ItemsFilter_);
		ItemsTable_->mouseWentUp ().connect (this, &AggregatorApp::HandleItemClicked);
		ItemsTable_->setAlternatingRowColors (true);
		ItemsTable_->setColumnWidth (0, { 550, Wt::LengthUnit::Pixel });
		ItemsTable_->setSelectionMode (Wt::SelectionMode::Single);
		ItemsTable_->setAttributeValue ("oncontextmenu",
				"event.cancelBubble = true; event.returnValue = false; return false;");

		auto itemPanel = rightPaneLay->addWidget (std::make_unique<Wt::WPanel> (), 5);

		auto scrollArea = itemPanel->setCentralWidget (std::make_unique<Wt::WContainerWidget> ());
		scrollArea->setOverflow (Wt::Overflow::Scroll, Wt::Orientation::Vertical);

		ItemView_ = scrollArea->addWidget (std::make_unique<Wt::WText> ());
		ItemView_->setTextFormat (Wt::TextFormat::UnsafeXHTML);
	}
}
}
}
