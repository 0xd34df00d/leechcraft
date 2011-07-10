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

#ifndef PLUGINS_AZOTH_PLUGINS_MODNOK_MODNOK_H
#define PLUGINS_AZOTH_PLUGINS_MODNOK_MODNOK_H
#include <QObject>
#include <QRegExp>
#include <QCache>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>

class QTranslator;
class QImage;

namespace LeechCraft
{
namespace Azoth
{
namespace Modnok
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)
		
		boost::shared_ptr<QTranslator> Translator_;

		QString ConvScriptPath_;

		QCache<QString, QImage> FormulasCache_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
	private:
		QImage GetRenderedImage (const QString&);
		QString HandleBody (QString);
	public slots:
		void hookFormatBodyEnd (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QString body,
				QObject *message);
	private slots:
		void clearCaches ();
	};
}
}
}

#endif
