/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Yury Erik Potapov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_GMAILNOTIFIER_GMAILNOTIFIER_H
#define PLUGINS_GMAILNOTIFIER_GMAILNOTIFIER_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iquarkcomponentprovider.h>
#include "convinfo.h"

class QTranslator;
class QTimer;

namespace LC
{
namespace GmailNotifier
{
	class Notifier;
	class GmailChecker;

	class GmailNotifier : public QObject
						, public IInfo
						, public IHaveSettings
						, public IQuarkComponentProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IQuarkComponentProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.GMailNotifier")

		Util::XmlSettingsDialog_ptr SettingsDialog_;

		GmailChecker *GmailChecker_;
		QTimer *UpdateTimer_;

		Notifier *Notifier_;

		QuarkComponent_ptr Quark_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QuarkComponents_t GetComponents () const;
	private slots:
		void setAuthorization ();
		void applyInterval ();
	};
}
}

#endif
