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
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <util/threads/coro/corocontext.h>

class QProcess;

namespace LC::Fenet
{
	class WMFinder;
	class CompFinder;
	class CompParamsManager;
	struct CompInfo;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Fenet")

		WMFinder *Finder_;

		CompFinder *CompFinder_;
		CompParamsManager *CompParamsManager_;

		Util::XmlSettingsDialog_ptr XSD_;

		QProcess *Process_ = nullptr;
		QProcess *CompProcess_ = nullptr;

		std::unique_ptr<Util::CoroContext> PendingWMStart_;
		std::unique_ptr<Util::CoroContext> PendingCompStart_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;
	private:
		void StartWM ();
		void RunSelectedWM ();

		CompInfo GetCompInfo (const QString&) const;

		void StartComp ();
		void RunSelectedComp ();
	private slots:
		void restartWM ();
		void restartComp ();
		void updateCompParamsManager (const QString&);
	};
}
