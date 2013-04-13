/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <QObject>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/icoreproxy.h>

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
enum class ActionsEmbedPlace;

namespace SB2
{
	class ActionImageProvider;
	class SBView;

	class BaseActionComponent : public QObject
	{
		Q_OBJECT
	protected:
		ICoreProxy_ptr Proxy_;
		QStandardItemModel *Model_;
		QuarkComponent_ptr Component_;

		ActionImageProvider *ImageProv_;

		int NextActionId_;
	public:
		enum class ActionPos
		{
			Beginning,
			End
		};
	protected:
		SBView *View_;

		const struct ComponentInfo
		{
			QString ImageProvID_;
			QString Filename_;
			QString ModelName_;
		} ComponentInfo_;
	public:
		BaseActionComponent (const ComponentInfo& info, ICoreProxy_ptr, SBView*, QObject* parent = 0);

		QuarkComponent_ptr GetComponent () const;

		virtual void AddActions (QList<QAction*>, ActionPos);
		virtual void RemoveAction (QAction*);
	protected:
		QStandardItem* FindItem (QAction*) const;
	protected slots:
		void handleActionDestroyed ();
		void handleActionChanged ();
	};
}
}
