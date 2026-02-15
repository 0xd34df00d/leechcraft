/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>

namespace LC::Summary
{
	class SummaryWidget;

	class Summary
		: public QObject
		, public IInfo
		, public IHaveTabs
		, public IHaveRecoverableTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveRecoverableTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.Summary")

		QPointer<SummaryWidget> Current_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		void RecoverTabs (const QList<TabRecoverInfo>&) override;
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const override;
	};
}
