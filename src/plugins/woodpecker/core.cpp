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


#include "core.h"
#include "twitterpage.h"

namespace LeechCraft
{
namespace Woodpecker
{
Core::Core ()
{
    TabClass_.TabClass_ = "Woodpecker";
    TabClass_.VisibleName_ = tr ("Twitter client");
    TabClass_.Description_ = tr ("The Woodpecker twitter client");
    TabClass_.Icon_ = QIcon (":/resources/images/woodpecker.svg");
    TabClass_.Priority_ = 70;
    TabClass_.Features_ = TFOpenableByRequest;
}

Core& Core::Instance ()
{
    static Core c;
    return c;
}

TabClassInfo Core::GetTabClass () const
{
    return TabClass_;
}

void Core::SetProxy (ICoreProxy_ptr proxy)
{
    Proxy_ = proxy;
}

ICoreProxy_ptr Core::GetProxy () const
{
    return Proxy_;
}

TwitterPage* Core::NewTabRequested ()
{
    TwitterPage *page = new TwitterPage ();//MakeTwitterPage ();
    emit addNewTab ("Woodpecker", page);
    emit raiseTab (page);
    emit changeTabIcon (page, QIcon (":/resources/images/woodpecker.svg"));

    return page;
}

void Core::Handle (const Entity& e)
{
    TwitterPage *page = NewTabRequested ();
//              page->SetText (e.Entity_.toString ());

    QString language = e.Additional_ ["Language"].toString ();
//              bool isTempDocumnet = e.Additional_ ["IsTemporaryDocument"].toBool ();
//              if (!language.isEmpty ())
//                  page->SetLanguage (language);
//              page->SetTemporaryDocument (isTempDocumnet);
}

TwitterPage* Core::MakeTwitterPage ()
{
    TwitterPage *result = new TwitterPage ();
    connect (result,
             SIGNAL (removeTab (QWidget*)),
             this,
             SIGNAL (removeTab (QWidget*)));
    connect (result,
             SIGNAL (changeTabName (QWidget*, const QString&)),
             this,
             SIGNAL (changeTabName (QWidget*, const QString&)));
    connect (result,
             SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
             this,
             SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)));
    connect (result,
             SIGNAL (delegateEntity (const LeechCraft::Entity&,
                                     int*, QObject**)),
             this,
             SIGNAL (delegateEntity (const LeechCraft::Entity&,
                                     int*, QObject**)));
    connect (result,
             SIGNAL (gotEntity (const LeechCraft::Entity&)),
             this,
             SIGNAL (gotEntity (const LeechCraft::Entity&)));
    return result;
}
};
};
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
