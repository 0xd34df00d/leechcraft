/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "traycomponent.h"
#include <QStandardItemModel>
#include <QAction>
#include <QDeclarativeImageProvider>
#include <QApplication>
#include <QDockWidget>
#include <util/sys/paths.h>
#include <util/util.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iactionsexporter.h>
#include "widthiconprovider.h"

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		class TrayModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				ActionObject = Qt::UserRole + 1,
				ActionText,
				ActionIcon
			};

			TrayModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::ActionObject] = "actionObject";
				roleNames [Roles::ActionText] = "actionText";
				roleNames [Roles::ActionIcon] = "actionIcon";
				setRoleNames (roleNames);
			}
		};
	}

	class ActionImageProvider : public WidthIconProvider
	{
		ICoreProxy_ptr Proxy_;
		QHash<int, QAction*> Actions_;
	public:
		ActionImageProvider (ICoreProxy_ptr proxy)
		: Proxy_ (proxy)
		{
		}

		QIcon GetIcon (const QStringList& list)
		{
			const auto id = list.at (0).toInt ();
			if (!Actions_.contains (id))
			{
				qWarning () << Q_FUNC_INFO
						<< "id not found:"
						<< id;
				return QIcon ();
			}

			auto act = Actions_ [id];
			auto icon = act->icon ();
			if (icon.isNull ())
				icon = Proxy_->GetIcon (act->property ("ActionIcon").toString (),
						act->property ("ActionIconOff").toString ());
			return icon;
		}

		void SetAction (int id, QAction *act)
		{
			Actions_ [id] = act;
		}

		void RemoveAction (QAction *act)
		{
			for (auto key : Actions_.keys (act))
				Actions_.remove (key);
		}
	};

	namespace
	{
		const QString ImageProviderID = "SB2_TrayActionImage";
	}

	TrayComponent::TrayComponent (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Model_ (new TrayModel (this))
	, ImageProv_ (new ActionImageProvider (proxy))
	, NextActionId_ (0)
	{
		Component_.Url_ = Util::GetSysPath (Util::SysPath::QML, "sb2", "TrayComponent.qml");
		Component_.DynamicProps_ << QPair<QString, QObject*> ("SB2_trayModel", Model_);
		Component_.ImageProviders_ << QPair<QString, QDeclarativeImageProvider*> (ImageProviderID, ImageProv_);

		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IActionsExporter*> ();
		for (QObject *actObj : hasActions)
			connect (actObj,
					SIGNAL (gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)),
					this,
					SLOT (handleGotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace)));
	}

	QuarkComponent TrayComponent::GetComponent () const
	{
		return Component_;
	}

	void TrayComponent::HandleDock (QDockWidget *dw, bool visible)
	{
		QAction *act = dw->toggleViewAction ();
		if (!visible)
			RemoveAction (act);
		else
			AddActions ({ act }, ActionPos::Beginning);
	}

	QStandardItem* TrayComponent::FindItem (QAction *action) const
	{
		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			auto item = Model_->item (i);
			const auto& objVar = item->data (TrayModel::Roles::ActionObject);
			if (objVar.value<QObject*> () == action)
				return item;
		}
		return 0;
	}

	void TrayComponent::AddActions (const QList<QAction*>& acts, ActionPos pos)
	{
		if (acts.isEmpty ())
			return;

		const auto& prefix = "image://" + ImageProviderID + '/';

		QMetaObject::invokeMethod (Model_, "modelAboutToBeReset");
		Model_->blockSignals (true);

		int insRow = 0;
		switch (pos)
		{
		case ActionPos::Beginning:
			insRow = 0;
			break;
		case ActionPos::End:
			insRow = Model_->rowCount ();
			break;
		}

		for (auto act : acts)
		{
			connect (act,
					SIGNAL (destroyed ()),
					this,
					SLOT (handleActionDestroyed ()));
			connect (act,
					SIGNAL (changed ()),
					this,
					SLOT (handleActionChanged ()));
			ImageProv_->SetAction (NextActionId_, act);

			const auto& idStr = QString::number (NextActionId_);

			auto item = new QStandardItem;
			item->setData (act->text (), TrayModel::Roles::ActionText);
			item->setData (prefix + idStr + "/0", TrayModel::Roles::ActionIcon);
			item->setData (QVariant::fromValue<QObject*> (act), TrayModel::Roles::ActionObject);
			Model_->insertRow (insRow++, item);

			++NextActionId_;
		}

		Model_->blockSignals (false);
		QMetaObject::invokeMethod (Model_, "modelReset");
	}

	void TrayComponent::RemoveAction (QAction *action)
	{
		if (auto item = FindItem (action))
			Model_->removeRow (item->row ());

		ImageProv_->RemoveAction (action);
	}

	void TrayComponent::handleGotActions (const QList<QAction*>& acts, ActionsEmbedPlace aep)
	{
		if (aep != ActionsEmbedPlace::LCTray && aep != ActionsEmbedPlace::QuickLaunch)
			return;

		AddActions (acts, ActionPos::End);
	}

	void TrayComponent::handleActionDestroyed ()
	{
		auto obj = sender ();
		RemoveAction (static_cast<QAction*> (obj));
	}

	void TrayComponent::handleActionChanged ()
	{
		auto item = FindItem (static_cast<QAction*> (sender ()));
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender not found"
					<< sender ();
			return;
		}

		auto str = item->data (TrayModel::Roles::ActionIcon).toString ();
		const int lastSlash = str.lastIndexOf ('/');
		const auto& uncacheStr = str.mid (lastSlash + 1);
		str.replace (lastSlash + 1, uncacheStr.size (), QString::number (uncacheStr.toInt () + 1));
		item->setData (str, TrayModel::Roles::ActionIcon);
	}

	void TrayComponent::handlePluginsAvailable ()
	{
		const auto places = { ActionsEmbedPlace::QuickLaunch, ActionsEmbedPlace::LCTray };
		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IActionsExporter*> ();
		for (auto place : places)
			for (auto exp : hasActions)
				handleGotActions (exp->GetActions (place), place);
	}
}
}
