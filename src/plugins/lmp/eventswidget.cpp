/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eventswidget.h"
#include <QStandardItemModel>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQuickItem>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/ieventsprovider.h>
#include <interfaces/iinfo.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/standardnamfactory.h>
#include <util/sll/udls.h>
#include <util/sys/paths.h>
#include <util/models/rolenamesmixin.h>
#include <util/threads/futures.h>
#include "literals.h"
#include "xmlsettingsmanager.h"
#include "util.h"

namespace LC::LMP
{
	namespace
	{
		// TODO replace with RoledItemsModel when we get events page scraping working
		class EventsModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Role
			{
				EventID = Qt::UserRole + 1,
				EventName,
				ImageThumbURL,
				ImageBigURL,
				Tags,
				Date,
				City,
				Place,
				Headliner,
				OtherArtists,
				CanBeAttended,
				IsAttended
			};

			explicit EventsModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> names;
				names [EventID] = "eventID";
				names [EventName] = "eventName";
				names [ImageThumbURL] = "eventImageThumbURL";
				names [ImageBigURL] = "eventImageBigURL";
				names [Tags] = "eventTags";
				names [Date] = "eventDate";
				names [City] = "eventCity";
				names [Place] = "eventPlace";
				names [Headliner] = "eventHeadliner";
				names [OtherArtists] = "eventArtists";
				names [CanBeAttended] = "canBeAttended";
				names [IsAttended] = "isAttended";
				setRoleNames (names);
			}
		};
	}

	EventsWidget::EventsWidget (QWidget *parent)
	: QWidget (parent)
	, View_ (new QQuickWidget)
	, Model_ (new EventsModel (this))
	{
		Ui_.setupUi (this);
		connect (Ui_.Provider_,
				&QComboBox::activated,
				this,
				&EventsWidget::RequestEvents);

		layout ()->addWidget (View_);

		new Util::StandardNAMFactory (Lits::LmpSlashQml,
				[] { return 50_mib; },
				View_->engine ());

		View_->setResizeMode (QQuickWidget::SizeRootObjectToView);

		View_->rootContext ()->setContextProperties ({
				{ QStringLiteral ("eventsModel"), QVariant::fromValue<QObject*> (Model_) },
				{ QStringLiteral ("eventsProxy"), QVariant::fromValue<QObject*> (this) },
				{ QStringLiteral ("attendSureTextString"), tr ("Sure!") },
				{ QStringLiteral ("attendMaybeTextString"), tr ("Maybe") },
				{ QStringLiteral ("unattendTextString"), tr ("Unattend") },
				{
					QStringLiteral ("colorProxy"),
					QVariant::fromValue<QObject*> (new Util::ColorThemeProxy (GetProxyHolder ()->GetColorThemeManager (), this))
				},
			});

		View_->setSource (Util::GetSysPathUrl (Util::SysPath::QML,
				Lits::LmpQmlSubdir,
				QStringLiteral ("EventsView.qml")));
	}

	void EventsWidget::InitializeProviders ()
	{
		const auto& lastProv = ShouldRememberProvs () ?
				XmlSettingsManager::Instance ()
					.Property ("LastUsedEventsProvider", QString ()).toString () :
				QString ();

		bool lastFound = false;

		const auto& roots = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableRoots<Media::IEventsProvider*> ();
		for (auto root : roots)
		{
			auto scrob = qobject_cast<Media::IEventsProvider*> (root);
			if (!scrob)
				continue;

			const auto& icon = qobject_cast<IInfo*> (root)->GetIcon ();
			Ui_.Provider_->addItem (icon, scrob->GetServiceName ());
			Providers_ << qobject_cast<Media::IEventsProvider*> (root);

			if (scrob->GetServiceName () == lastProv)
			{
				const int idx = Providers_.size () - 1;
				Ui_.Provider_->setCurrentIndex (idx);
				RequestEvents (idx);
				lastFound = true;
			}
		}

		if (!lastFound)
			Ui_.Provider_->setCurrentIndex (-1);
	}

	void EventsWidget::RequestEvents (int index)
	{
		Model_->clear ();

		auto prov = Providers_.at (index);
		Util::Sequence (this, prov->UpdateRecommendedEvents ()) >>
				Util::Visitor
				{
					[] (const QString&) { /* TODO */ },
					[this, index] (const Media::EventInfos_t& events)
					{
						if (index == Ui_.Provider_->currentIndex ())
							HandleEvents (events);
					}
				};

		XmlSettingsManager::Instance ().setProperty ("LastUsedEventsProvider", prov->GetServiceName ());
	}

	void EventsWidget::HandleEvents (const Media::EventInfos_t& events)
	{
		Model_->clear ();

		for (const auto& event : events)
		{
			auto item = new QStandardItem;
			item->setData (event.ID_, EventsModel::Role::EventID);
			item->setData (event.Name_, EventsModel::Role::EventName);
			item->setData (event.SmallImage_, EventsModel::Role::ImageThumbURL);
			item->setData (event.BigImage_, EventsModel::Role::ImageBigURL);
			item->setData (event.Tags_.join (u"; "), EventsModel::Role::Tags);
			item->setData (QLocale {}.toString (event.Date_, QLocale::LongFormat), EventsModel::Role::Date);
			item->setData (event.PlaceName_, EventsModel::Role::Place);
			item->setData (event.City_, EventsModel::Role::City);

			if (!event.Headliner_.isEmpty ())
				item->setData (tr ("Headliner: %1").arg (event.Headliner_),
						EventsModel::Role::Headliner);

			auto otherArtists = event.Artists_;
			otherArtists.removeAll (event.Headliner_);
			item->setData (otherArtists.isEmpty () ?
						QString () :
						tr ("Other artists: %1").arg (otherArtists.join (u"; ")),
					EventsModel::Role::OtherArtists);

			item->setData (event.CanBeAttended_, EventsModel::Role::CanBeAttended);
			item->setData (event.AttendType_ != Media::EventAttendType::None,
					EventsModel::Role::IsAttended);

			Model_->appendRow (item);
		}
	}

	void EventsWidget::attendSure (int id)
	{
		if (auto prov = Providers_.value (Ui_.Provider_->currentIndex ()))
			prov->AttendEvent (id, Media::EventAttendType::Surely);
	}

	void EventsWidget::attendMaybe (int id)
	{
		if (auto prov = Providers_.value (Ui_.Provider_->currentIndex ()))
			prov->AttendEvent (id, Media::EventAttendType::Maybe);
	}

	void EventsWidget::unattend (int id)
	{
		if (auto prov = Providers_.value (Ui_.Provider_->currentIndex ()))
			prov->AttendEvent (id, Media::EventAttendType::None);
	}
}
