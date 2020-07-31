/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/idatafilter.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/data/iimgsource.h>

namespace LC
{
namespace Blasq
{
	class ServicesManager;
	class AccountsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IHaveRecoverableTabs
				 , public IPluginReady
				 , public IHaveSettings
				 , public IImgSource
				 , public IEntityHandler
				 , public IDataFilter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveRecoverableTabs IPluginReady IHaveSettings IImgSource IEntityHandler IDataFilter)

		LC_PLUGIN_METADATA ("org.LeechCraft.Blasq")

		ICoreProxy_ptr Proxy_;

		ServicesManager *ServicesMgr_;
		AccountsManager *AccountsMgr_;
		Util::XmlSettingsDialog_ptr XSD_;

		TabClassInfo PhotosTabTC_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		void RecoverTabs (const QList<TabRecoverInfo>&);
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		ImageServiceInfos_t GetServices () const;
		IPendingImgSourceRequest* RequestImages (const QByteArray& serviceId);
		IPendingImgSourceRequest* StartDefaultChooser ();

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		QString GetFilterVerb () const;
		QList<FilterVariant> GetFilterVariants (const QVariant&) const;
	private:
		void TabOpenRequested (const QByteArray&, const DynPropertiesList_t&, QDataStream* = nullptr);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
}
}

