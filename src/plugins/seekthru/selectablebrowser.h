/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_SEEKTHRU_SELECTABLEBROWSER_H
#define PLUGINS_SEEKTHRU_SELECTABLEBROWSER_H
#include <QWidget>
#include <memory>
#include <QTextBrowser>
#include <interfaces/iwebbrowser.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class SelectableBrowser : public QWidget
			{
				Q_OBJECT

				bool Internal_;
				std::auto_ptr<QTextBrowser> InternalBrowser_;
				std::auto_ptr<IWebWidget> ExternalBrowser_;
			public:
				SelectableBrowser (QWidget* = 0);
				void Construct (IWebBrowser*);

				void SetHtml (const QString&, const QUrl& = QUrl ());
			};
		};
	};
};

#endif

