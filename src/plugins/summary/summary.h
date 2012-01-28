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

#pragma once

#include <memory>
#include <QObject>
#include <QStringList>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/isummaryrepresentation.h>

namespace LeechCraft
{
namespace Summary
{
	class Summary : public QObject
					, public IInfo
					, public IHaveTabs
					, public IEntityHandler
					, public ISummaryRepresentation
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IEntityHandler ISummaryRepresentation)

		std::auto_ptr<QTranslator> Translator_;
		TabClasses_t TabClasses_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Provides () const;
		QStringList Needs () const;
		QStringList Uses () const;
		void SetProvider (QObject*, const QString&);

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		EntityTestHandleResult CouldHandle (const LeechCraft::Entity&) const;
		void Handle (LeechCraft::Entity);

		QModelIndex MapToSource (const QModelIndex&) const;
		QTreeView* GetCurrentView () const;
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
