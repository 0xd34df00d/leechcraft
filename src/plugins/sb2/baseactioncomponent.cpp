/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "baseactioncomponent.h"
#include <QStandardItemModel>
#include <QAction>
#include <QApplication>
#include <util/sys/paths.h>
#include <util/util.h>
#include <util/qml/widthiconprovider.h>
#include <util/models/rolenamesmixin.h>
#include <interfaces/core/iiconthememanager.h>
#include "sbview.h"

namespace LC
{
namespace SB2
{
	namespace
	{
		class TrayModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Roles
			{
				ActionObject = Qt::UserRole + 1,
				ActionText,
				ActionIcon
			};

			TrayModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::ActionObject] = "actionObject";
				roleNames [Roles::ActionText] = "actionText";
				roleNames [Roles::ActionIcon] = "actionIcon";
				setRoleNames (roleNames);
			}
		};
	}

	class ActionImageProvider : public Util::WidthIconProvider
	{
		ICoreProxy_ptr Proxy_;
		QHash<int, QAction*> Actions_;
	public:
		ActionImageProvider (ICoreProxy_ptr proxy)
		: Proxy_ (proxy)
		{
		}

		ActionImageProvider (const ActionImageProvider&) = delete;
		ActionImageProvider (ActionImageProvider&&) = delete;

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
			{
				const auto mgr = Proxy_->GetIconThemeManager ();
				icon = mgr->GetIcon (act->property ("ActionIcon").toString (),
						act->property ("ActionIconOff").toString ());
			}
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

	BaseActionComponent::BaseActionComponent (const ComponentInfo& info, ICoreProxy_ptr proxy, SBView *view, QObject* parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Model_ (new TrayModel (this))
	, Component_ (new QuarkComponent ("sb2", info.Filename_))
	, ImageProv_ (new ActionImageProvider (proxy))
	, View_ (view)
	, ComponentInfo_ (info)
	{
		Component_->DynamicProps_.append ({ info.ModelName_, Model_ });
		Component_->ImageProviders_.append ({ info.ImageProvID_, ImageProv_ });
	}

	QuarkComponent_ptr BaseActionComponent::GetComponent () const
	{
		return Component_;
	}

	void BaseActionComponent::AddActions (QList<QAction*> acts, ActionPos pos)
	{
		for (auto act : QList<QAction*> (acts))
			if (FindItem (act))
			{
				qWarning () << Q_FUNC_INFO
						<< "duplicate action inserted"
						<< act;
				acts.removeAll (act);
			}
		if (acts.isEmpty ())
			return;

		const auto& prefix = "image://" + ComponentInfo_.ImageProvID_ + '/';

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
			View_->addAction (act);
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
			item->setData (act->toolTip ().isEmpty () ? act->text () : act->toolTip (),
					TrayModel::Roles::ActionText);
			item->setData (prefix + idStr + "/0", TrayModel::Roles::ActionIcon);
			item->setData (QVariant::fromValue<QObject*> (act), TrayModel::Roles::ActionObject);
			Model_->insertRow (insRow++, item);

			++NextActionId_;
		}
	}

	void BaseActionComponent::RemoveAction (QAction *action)
	{
		if (auto item = FindItem (action))
			Model_->removeRow (item->row ());

		ImageProv_->RemoveAction (action);

		disconnect (action,
				0,
				this,
				0);
	}

	QStandardItem* BaseActionComponent::FindItem (QAction *action) const
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

	void BaseActionComponent::handleActionDestroyed ()
	{
		auto obj = sender ();
		RemoveAction (static_cast<QAction*> (obj));
	}

	void BaseActionComponent::handleActionChanged ()
	{
		auto act = static_cast<QAction*> (sender ());
		auto item = FindItem (act);
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

		item->setData (act->toolTip ().isEmpty () ? act->text () : act->toolTip (),
				TrayModel::Roles::ActionText);
	}
}
}
