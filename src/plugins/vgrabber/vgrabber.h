/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_VGRABBER_VGRABBER_H
#define PLUGINS_VGRABBER_VGRABBER_H
#include <memory>
#include <QObject>
#include <QStringList>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			class CategoriesSelector;

			class vGrabber : public QObject
						   , public IInfo
						   , public IFinder
						   , public IHaveSettings
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IFinder IHaveSettings)

				std::auto_ptr<QTranslator> Translator_;
				boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
				ICoreProxy_ptr Proxy_;

				CategoriesSelector *Audio_;
				CategoriesSelector *Video_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				QStringList GetCategories () const;
				QList<IFindProxy_ptr> GetProxy (const Request&);

				boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;

				ICoreProxy_ptr GetProxy () const;
			private slots:
				void handleError (const QString&);
				void handleCategoriesGoingToChange (const QStringList&,
						const QStringList&);
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
				void categoriesChanged (const QStringList&, const QStringList&);
			};
		};
	};
};

#endif

