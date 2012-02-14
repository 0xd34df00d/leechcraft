/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Rayslava <rayslava@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PLUGINS_WOODPECKER_CORE_H
#define PLUGINS_WOODPECKER_CORE_H

#include <QObject>
#include <QIcon>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include <interfaces/ihavetabs.h>
#include "twitterpage.h"

namespace LeechCraft
{
namespace Woodpecker
{
class TwitterPage;

class Core : public QObject
{
	Q_OBJECT

	ICoreProxy_ptr Proxy_;
	TabClassInfo TabClass_;

	Core ();
public:
	static Core& Instance ();
	TabClassInfo GetTabClass () const;

	void SetProxy (ICoreProxy_ptr);
	ICoreProxy_ptr GetProxy () const;

	TwitterPage* NewTabRequested ();
	void Handle (const Entity&);
private:
	TwitterPage* MakeTwitterPage ();
signals:
	void addNewTab (const QString&, QWidget*);
	void removeTab (QWidget*);
	void changeTabName (QWidget*, const QString&);
	void changeTabIcon (QWidget*, const QIcon&);
	void changeTooltip (QWidget*, QWidget*);
	void statusBarChanged (QWidget*, const QString&);
	void raiseTab (QWidget*);
	void delegateEntity (const LeechCraft::Entity&,
						 int*, QObject**);
	void gotEntity (const LeechCraft::Entity&);

	void couldHandle (const LeechCraft::Entity&, bool*);
};
};
};

#endif // PLUGINS_WOODPECKER_CORE_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
