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
#include <util/sys/paths.h>
#include <util/util.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iactionsexporter.h>

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
				ActionText = Qt::UserRole + 1,
				ActionIcon
			};

			TrayModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::ActionText] = "actionText";
				roleNames [Roles::ActionIcon] = "actionIcon";
				setRoleNames (roleNames);
			}
		};
	}

	class ActionImageProvider : public QDeclarativeImageProvider
	{
		ICoreProxy_ptr Proxy_;
		QHash<int, QAction*> Actions_;
	public:
		ActionImageProvider (ICoreProxy_ptr proxy)
		: QDeclarativeImageProvider (Pixmap)
		, Proxy_ (proxy)
		{
		}

		QPixmap requestPixmap (const QString& idStr, QSize *size, const QSize& requestedSize)
		{
			const auto& list = idStr.split ('/', QString::SkipEmptyParts);
			if (list.isEmpty ())
				return QPixmap ();

			const auto id = list.at (0).toInt ();
			if (!Actions_.contains (id))
			{
				qWarning () << Q_FUNC_INFO
						<< "id not found:"
						<< id;
				return QPixmap ();
			}

			auto realSize = requestedSize;
			if (realSize.width () <= 0)
			{
				const int width = list.last ().toInt ();
				realSize = QSize (width, width);
			}

			auto act = Actions_ [id];
			auto icon = act->icon ();
			if (icon.isNull ())
				icon = Proxy_->GetIcon (act->property ("ActionIcon").toString (),
						act->property ("ActionIconOff").toString ());

			if (size)
				*size = icon.actualSize (realSize);

			return icon.pixmap (realSize);
		}

		void SetAction (int id, QAction *act)
		{
			Actions_ [id] = act;
		}
	};

	const QString ImageProviderID = "LeechCraft_SB2_TrayActionImage";

	TrayComponent::TrayComponent (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Model_ (new TrayModel (this))
	, ImageProv_ (new ActionImageProvider (proxy))
	, NextActionId_ (0)
	{
		Component_.Url_ = Util::GetSysPath (Util::SysPath::QML, "sb2", "TrayComponent.qml");
		Component_.DynamicProps_ << QPair<QString, QObject*> ("trayModel", Model_);
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

	void TrayComponent::handleGotActions (const QList<QAction*>& acts, ActionsEmbedPlace aep)
	{
		if (acts.isEmpty () || aep != ActionsEmbedPlace::LCTray)
			return;

		const auto& prefix = "image://" + ImageProviderID + '/';

		QMetaObject::invokeMethod (Model_, "modelAboutToBeReset");
		Model_->blockSignals (true);

		for (auto act : acts)
		{
			ImageProv_->SetAction (NextActionId_, act);

			const auto& idStr = QString::number (NextActionId_);

			auto item = new QStandardItem;
			item->setData (act->text (), TrayModel::Roles::ActionText);
			item->setData (prefix + idStr + "/0", TrayModel::Roles::ActionIcon);
			Model_->appendRow (item);

			++NextActionId_;
		}

		Model_->blockSignals (false);
		QMetaObject::invokeMethod (Model_, "modelReset");
	}

	void TrayComponent::handlePluginsAvailable ()
	{
		const auto& hasActions = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IActionsExporter*> ();
		for (auto exp : hasActions)
			handleGotActions (exp->GetActions (ActionsEmbedPlace::LCTray),
					ActionsEmbedPlace::LCTray);
	}
}
}
