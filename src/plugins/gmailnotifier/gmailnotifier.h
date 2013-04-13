/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Yury Erik Potapov
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

#ifndef PLUGINS_GMAILNOTIFIER_GMAILNOTIFIER_H
#define PLUGINS_GMAILNOTIFIER_GMAILNOTIFIER_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iquarkcomponentprovider.h>
#include "convinfo.h"

class QTranslator;
class QTimer;

namespace LeechCraft
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
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}

#endif
