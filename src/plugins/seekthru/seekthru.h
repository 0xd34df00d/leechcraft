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
#include <interfaces/ifinder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/idatafilter.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/isyncable.h>
#include <interfaces/structures.h>

namespace LC::SeekThru
{
	class SeekThru : public QObject
					, public IInfo
					, public IFinder
					, public IHaveSettings
					, public IEntityHandler
					, public IDataFilter
					, public IStartupWizard
					, public ISyncable
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IFinder
				IHaveSettings
				IEntityHandler
				IDataFilter
				IStartupWizard
				ISyncable)

		LC_PLUGIN_METADATA ("org.LeechCraft.SeekThru")

		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;
		QStringList Provides () const override;
		QStringList Needs () const override;
		QStringList Uses () const override;
		void SetProvider (QObject*, const QString&) override;

		QStringList GetCategories () const override;
		QList<IFindProxy_ptr> GetProxy (const LC::Request&) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		EntityTestHandleResult CouldHandle (const LC::Entity&) const override;
		void Handle (LC::Entity) override;

		QString GetFilterVerb () const override;
		QList<FilterVariant> GetFilterVariants (const QVariant&) const override;

		QList<QWizardPage*> GetWizardPages () const override;

		ISyncProxy* GetSyncProxy () override;
	private slots:
		void handleError (const QString&);
	signals:
		void categoriesChanged (const QStringList&, const QStringList&) override;
	};
}
