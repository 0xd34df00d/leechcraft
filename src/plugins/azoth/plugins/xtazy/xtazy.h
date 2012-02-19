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

#ifndef PLUGINS_AZOTH_PLUGINS_XTAZY_XTAZY_H
#define PLUGINS_AZOTH_PLUGINS_XTAZY_XTAZY_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>

class QTranslator;

namespace LeechCraft
{
namespace Azoth
{
class IProxyObject;

namespace Xtazy
{
	class TuneSourceBase;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		std::shared_ptr<QTranslator> Translator_;
		ICoreProxy_ptr Proxy_;
		IProxyObject *AzothProxy_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
		QList<TuneSourceBase*> TuneSources_;
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
	public slots:
		void initPlugin (QObject*);
	private slots:
		void publish (const QMap<QString, QVariant>&);
	};
}
}
}

#endif
