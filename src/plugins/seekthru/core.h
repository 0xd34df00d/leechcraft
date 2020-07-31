/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QMap>
#include <interfaces/structures.h>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/isyncable.h>
#include "description.h"
#include "searchhandler.h"

class IWebBrowser;

namespace LC::SeekThru
{
	class Core : public QAbstractItemModel
	{
		Q_OBJECT

		QMap<QString, QObject*> Providers_;
		QList<Description> Descriptions_;
		QStringList Headers_;
		ICoreProxy_ptr Proxy_;

		static const QString OS_;

		Core ();
	public:
		enum Roles
		{
			RoleDescription = Qt::UserRole + 1,
			RoleContact,
			RoleTags,
			RoleLongName,
			RoleDeveloper,
			RoleAttribution,
			RoleRight,
			RoleAdult,
			RoleLanguages
		};

		static Core& Instance ();

		void DoDelayedInit ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		int columnCount (const QModelIndex& = QModelIndex ()) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = QModelIndex ()) const override;

		void SetProvider (QObject*, const QString&);
		bool CouldHandle (const Entity&) const;
		void Handle (const Entity&);

		/** Fetches the searcher from the url.
			*
			* @param[in] url The url with the search description.
			*/
		void Add (const QUrl& url);
		void Remove (const QModelIndex&);
		void SetTags (const QModelIndex&, const QStringList&);
		QStringList GetCategories () const;
		IFindProxy_ptr GetProxy (const LC::Request&);
		IWebBrowser* GetWebBrowser () const;
		void HandleEntity (const QString&, const QString& = QString ());
	private:
		void SetTags (int, const QStringList&);
		QStringList ComputeUniqueCategories () const;
		QList<Description> FindMatchingHRTag (const QString&) const;
		Description ParseData (const QString&, const QString&);
		void ReadSettings ();
		void WriteSettings ();
	public:
		bool HandleDADescrAdded (QDataStream&);
		bool HandleDADescrRemoved (QDataStream&);
		bool HandleDATagsChanged (QDataStream&);
	signals:
		void error (const QString&);
		void categoriesChanged (const QStringList&, const QStringList&);
	};
}
