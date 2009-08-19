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

#ifndef REMOTER_H
#define REMOTER_H
#include <QMainWindow>
#include <interfaces/iinfo.h>
#include <interfaces/iwindow.h>
#include "ui_mainwindow.h"

namespace LeechCraft
{
	namespace Util
	{
		class MergeModel;
	};
};

class Remoter : public QMainWindow
              , public IInfo
              , public IWindow
{
    Q_OBJECT
    Q_INTERFACES (IInfo IWindow);

    Ui::MainWindow Ui_;
    bool IsShown_;
public:
    void Init ();
    void Release ();
    QString GetName () const;
    QString GetInfo () const;
    QStringList Provides () const;
    QStringList Needs () const;
    QStringList Uses () const;
    void SetProvider (QObject*, const QString&);
    QIcon GetIcon () const;
    void SetParent (QWidget*);
    void ShowWindow ();
protected:
    virtual void closeEvent (QCloseEvent*);
public Q_SLOTS:
    void handleHidePlugins ();
	void pushHistoryModel (LeechCraft::Util::MergeModel*) const;
	void pushDownloadersModel (LeechCraft::Util::MergeModel*) const;
};

#endif

