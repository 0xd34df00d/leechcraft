/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/iquarkcomponentprovider.h>
#include "manifest.h"

class QTranslator;

namespace LC::SB2
{
	class ViewManager;
	class QuarkSettingsManager;

	class QuarkManager : public QObject
	{
		ViewManager * const ViewMgr_;
		ICoreProxy_ptr Proxy_;

		const QuarkComponent_ptr Component_;
		const QUrl URL_;

		Util::XmlSettingsDialog_ptr XSD_;
		QuarkSettingsManager *SettingsManager_ = nullptr;

		const std::shared_ptr<QTranslator> Translator_;

		const Manifest Manifest_;
	public:
		QuarkManager (QuarkComponent_ptr, ViewManager*);

		const Manifest& GetManifest () const;
		bool IsValidArea () const;

		bool HasSettings () const;
		Util::XmlSettingsDialog* GetXSD () const;
	private:
		QString GetSuffixedName (const QString&) const;
		std::shared_ptr<QTranslator> TryLoadTranslator () const;

		void CreateSettings ();
	};

	typedef std::shared_ptr<QuarkManager> QuarkManager_ptr;
}
