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
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaverecoverabletabs.h>

class QTranslator;

namespace LC
{
namespace Popishu
{
	class EditorPage;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IEntityHandler
				 , public IHaveSettings
				 , public IHaveRecoverableTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IEntityHandler IHaveSettings IHaveRecoverableTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.Popishu")

		TabClassInfo TabClass_;
		ICoreProxy_ptr Proxy_;
		std::shared_ptr<Util::XmlSettingsDialog> XmlSettingsDialog_;
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

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		std::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const override;

		void RecoverTabs (const QList<TabRecoverInfo>&) override;
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const override;
	private:
		EditorPage* MakeEditorPage ();
		void AnnouncePage (EditorPage*);
	signals:
		void addNewTab (const QString&, QWidget*) override;
		void removeTab (QWidget*) override;
		void changeTabName (QWidget*, const QString&) override;
		void changeTabIcon (QWidget*, const QIcon&) override;
		void statusBarChanged (QWidget*, const QString&) override;
		void raiseTab (QWidget*) override;
	};
}
}
