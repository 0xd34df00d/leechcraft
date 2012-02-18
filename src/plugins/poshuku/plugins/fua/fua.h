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

#ifndef PLUGINS_POSHUKU_PLUGINS_FUA_FUA_H
#define PLUGINS_POSHUKU_PLUGINS_FUA_FUA_H
#include <memory>
#include <QObject>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

class QStandardItemModel;
class QWebPage;

namespace LeechCraft
{
namespace Util
{
	class XmlSettingsDialog;
};
namespace Poshuku
{
namespace Fua
{
	class Settings;

	class FUA : public QObject
			  , public IInfo
			  , public IPlugin2
			  , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		std::shared_ptr<QStandardItemModel> Model_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		QMap<QString, QString> Browser2ID_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Provides () const;
		QStringList Needs () const;
		QStringList Uses () const;
		void SetProvider (QObject*, const QString&);

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		void Save () const;
		const QMap<QString, QString>& GetBrowser2ID () const;
	public slots:
		void hookUserAgentForUrlRequested (LeechCraft::IHookProxy_ptr,
				const QUrl&, const QWebPage*);
	};
}
}
}

#endif
