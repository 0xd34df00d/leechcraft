/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/icoreproxy.h>

class QStandardItemModel;
class QStandardItem;

namespace LC
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
		QStandardItemModel * const Model_;
		QuarkComponent_ptr Component_;

		ActionImageProvider * const ImageProv_;

		int NextActionId_ = 0;
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
