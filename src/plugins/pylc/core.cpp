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

#include "core.h"
#include <QDir>
#include <QFileInfo>
#include <PythonQt/PythonQt.h>
#include <PythonQt/PythonQtGui.h>
#include "wrapperobject.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PyLC
		{
			Core::Core ()
			{
				PythonQt::init (PythonQt::RedirectStdOut);
				PythonQtGui::init ();
				connect (PythonQt::self (),
						SIGNAL (pythonStdOut (const QString&)),
						this,
						SLOT (handleStdOut (const QString&)));
				connect (PythonQt::self (),
						SIGNAL (pythonStdErr (const QString&)),
						this,
						SLOT (handleStdErr (const QString&)));

				QStringList paths = FindPlugins ();
				Q_FOREACH (QString path, paths)
					Plugins_ << new WrapperObject (path);
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			QList<QObject*> Core::GetPlugins ()
			{
				return Plugins_;
			}

			QStringList Core::FindPlugins () const
			{
				struct Collector
				{
					QFileInfoList operator() (const QString& path)
					{
						QFileInfoList list;
						QDir dir = QDir::home ();
						if (dir.cd (path))
							list = dir.entryInfoList (QStringList ("*.py"),
									QDir::Files |
										QDir::NoDotAndDotDot |
										QDir::Readable,
									QDir::Name);
						return list;
					}
				};

				QFileInfoList list;
				list += Collector () (".leechcraft/plugins/python");

				QStringList result;
				Q_FOREACH (QFileInfo info, list)
					result += info.absoluteFilePath ();
				return result;
			}

			void Core::handleStdOut (const QString& str)
			{
				qDebug () << "<python>" << str;
			}

			void Core::handleStdErr (const QString& str)
			{
				qWarning () << "<python>" << str;
			}
		};
	};
};

