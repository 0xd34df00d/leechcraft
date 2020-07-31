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
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/isummaryrepresentation.h>
#include <interfaces/ihaverecoverabletabs.h>

namespace LC
{
namespace Summary
{
	class Summary : public QObject
					, public IInfo
					, public IHaveTabs
					, public ISummaryRepresentation
					, public IHaveRecoverableTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs ISummaryRepresentation IHaveRecoverableTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.Summary")

		TabClasses_t TabClasses_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		QModelIndex MapToSource (const QModelIndex&) const;
		QTreeView* GetCurrentView () const;

		void RecoverTabs (const QList<TabRecoverInfo>&);
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const;
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
}
}
