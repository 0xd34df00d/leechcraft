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

#ifndef PLUGINS_AZOTH_PLUGINS_AUTOIDLER_AUTOIDLER_H
#define PLUGINS_AZOTH_PLUGINS_AUTOIDLER_AUTOIDLER_H
#include <memory>
#include <QObject>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iclentry.h>
#include <interfaces/ilastactivityprovider.h>

class QTranslator;

class Idle;

namespace LeechCraft
{
namespace Azoth
{
class IProxyObject;

namespace Autoidler
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public ILastActivityProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings LeechCraft::Azoth::ILastActivityProvider)

		ICoreProxy_ptr Proxy_;
		IProxyObject *AzothProxy_;
		std::shared_ptr<QTranslator> Translator_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		std::shared_ptr<Idle> Idle_;

		QMap<QObject*, EntryStatus> OldStatuses_;

		int IdleSeconds_;
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

		int GetInactiveSeconds ();
	public slots:
		void initPlugin (QObject*);
	private slots:
		void handleIdle (int);
	};
}
}
}

#endif
