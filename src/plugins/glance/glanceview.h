/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

class QQuickWidget;
class QStandardItemModel;

class ICoreTabWidget;

namespace LC::Glance
{
	class ThumbsProvider;

	class GlanceView : public QObject
	{
		Q_OBJECT

		ICoreTabWidget& Tabs_;
		std::unique_ptr<QQuickWidget> View_;

		QStandardItemModel& ThumbsModel_;
		ThumbsProvider& ThumbsProvider_;
	public:
		explicit GlanceView (ICoreTabWidget&, QObject* = nullptr);
		~GlanceView () override;
	public slots:
		void finish ();
		void selectItem (int);
		int deleteItem (int);
	private:
		void Start ();
	};
}
