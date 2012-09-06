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

#include "eventswidget.h"
#include <QStandardItemModel>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QtDebug>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/ieventsprovider.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class EventsModel : public QStandardItemModel
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

			EventsModel (QObject *parent = 0)
			: QStandardItemModel (parent)
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
	, Model_ (new EventsModel (this))
	{
		Ui_.setupUi (this);

		Ui_.View_->rootContext ()->setContextProperty ("eventsModel", Model_);
		Ui_.View_->rootContext ()->setContextProperty ("attendSureTextString", tr ("Sure!"));
		Ui_.View_->rootContext ()->setContextProperty ("attendMaybeTextString", tr ("Maybe"));
		Ui_.View_->rootContext ()->setContextProperty ("unattendTextString", tr ("Unattend"));
		Ui_.View_->setSource (QUrl ("qrc:/lmp/resources/qml/EventsView.qml"));

		connect (Ui_.View_->rootObject (),
				SIGNAL (attendSure (int)),
				this,
				SLOT (handleAttendSure (int)));
		connect (Ui_.View_->rootObject (),
				SIGNAL (attendMaybe (int)),
				this,
				SLOT (handleAttendMaybe (int)));
		connect (Ui_.View_->rootObject (),
				SIGNAL (unattend (int)),
				this,
				SLOT (handleUnattend (int)));
	}

	void EventsWidget::InitializeProviders ()
	{
		const auto& lastProv = XmlSettingsManager::Instance ()
				.Property ("LastUsedEventsProvider", QString ()).toString ();

		bool lastFound = false;

		const auto& roots = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableRoots<Media::IEventsProvider*> ();
		Q_FOREACH (auto root, roots)
		{
			auto scrob = qobject_cast<Media::IEventsProvider*> (root);
			if (!scrob)
				continue;

			Ui_.Provider_->addItem (scrob->GetServiceName ());
			Providers_ << qobject_cast<Media::IEventsProvider*> (root);

			connect (root,
					SIGNAL (gotRecommendedEvents (Media::EventInfos_t)),
					this,
					SLOT (handleEvents (Media::EventInfos_t)));

			if (scrob->GetServiceName () == lastProv)
			{
				const int idx = Providers_.size () - 1;
				Ui_.Provider_->setCurrentIndex (idx);
				on_Provider__activated (idx);
				lastFound = true;
			}
		}

		if (!lastFound)
			Ui_.Provider_->setCurrentIndex (-1);
	}

	void EventsWidget::on_Provider__activated (int index)
	{
		Model_->clear ();

		auto prov = Providers_.at (index);
		prov->UpdateRecommendedEvents ();

		XmlSettingsManager::Instance ().setProperty ("LastUsedEventsProvider", prov->GetServiceName ());
	}

	void EventsWidget::handleEvents (const Media::EventInfos_t& events)
	{
		const int provIdx = Ui_.Provider_->currentIndex ();
		if (provIdx < 0 ||
			qobject_cast<Media::IEventsProvider*> (sender ()) != Providers_.value (provIdx))
			return;

		Model_->clear ();

		Q_FOREACH (const auto& event_t, events)
		{
			Media::EventInfo event (event_t);

			auto item = new QStandardItem;
			item->setData (event.ID_, EventsModel::Role::EventID);
			item->setData (event.Name_, EventsModel::Role::EventName);
			item->setData (event.SmallImage_, EventsModel::Role::ImageThumbURL);
			item->setData (event.BigImage_, EventsModel::Role::ImageBigURL);
			item->setData (event.Tags_.join ("; "), EventsModel::Role::Tags);
			item->setData (event.Date_.toString (Qt::SystemLocaleLongDate), EventsModel::Role::Date);
			item->setData (event.PlaceName_, EventsModel::Role::Place);
			item->setData (event.City_, EventsModel::Role::City);

			if (!event.Headliner_.isEmpty ())
				item->setData (tr ("Headliner: %1").arg (event.Headliner_),
						EventsModel::Role::Headliner);

			auto otherArtists = event.Artists_;
			otherArtists.removeAll (event.Headliner_);
			item->setData (otherArtists.isEmpty () ?
						QString () :
						tr ("Other artists: %1").arg (otherArtists.join ("; ")),
					EventsModel::Role::OtherArtists);

			item->setData (event.CanBeAttended_, EventsModel::Role::CanBeAttended);
			item->setData (event.AttendType_ != Media::EventAttendType::None,
					EventsModel::Role::IsAttended);

			Model_->appendRow (item);
		}
	}

	void EventsWidget::handleAttendSure (int id)
	{
		auto prov = Providers_.value (Ui_.Provider_->currentIndex ());
		if (!prov)
			return;
		prov->AttendEvent (id, Media::EventAttendType::Surely);
	}

	void EventsWidget::handleAttendMaybe (int id)
	{
		auto prov = Providers_.value (Ui_.Provider_->currentIndex ());
		if (!prov)
			return;
		prov->AttendEvent (id, Media::EventAttendType::Maybe);
	}

	void EventsWidget::handleUnattend (int id)
	{
		auto prov = Providers_.value (Ui_.Provider_->currentIndex ());
		if (!prov)
			return;
		prov->AttendEvent (id, Media::EventAttendType::None);
	}
}
}
