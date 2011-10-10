/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_AUTOPASTE_AUTOPASTE_H
#define PLUGINS_AZOTH_PLUGINS_AUTOPASTE_AUTOPASTE_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>

class QTranslator;

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		ICoreProxy_ptr Proxy_;
		boost::shared_ptr<QTranslator> Translator_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		QMap<QNetworkReply*, QPointer<QObject> > Reply2Entry_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private:
		void Paste (const QString&, QObject*);
	public slots:
		void hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant);
	private slots:
		void handleMetadata ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}

#endif
