/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/core/icoreproxyfwd.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/blogique/iblogiquesidewidget.h>
#include "ui_postoptionswidget.h"

namespace Media
{
	class ICurrentSongKeeper;
}

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJAccount;

	class PostOptionsWidget : public QWidget
							, public IBlogiqueSideWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blogique::IBlogiqueSideWidget)

		const ICoreProxy_ptr Proxy_;

		Ui::PostOptions Ui_;
		LJAccount *Account_ = nullptr;
		quint32 AllowMask_ = 0;

	public:
		explicit PostOptionsWidget (const ICoreProxy_ptr& proxy, QWidget *parent = nullptr);

		QString GetName () const override;
		SideWidgetType GetWidgetType () const override;
		QVariantMap GetPostOptions () const override;
		void SetPostOptions (const QVariantMap& map) override;
		QVariantMap GetCustomData () const override;
		void SetCustomData (const QVariantMap& map) override;
		void SetAccount (QObject *account) override;

	private:
		void FillItems ();

	public slots:
		void handleAutoUpdateCurrentMusic ();
	private slots:
		void on_Access__activated (int index);
		void on_UserPic__currentIndexChanged (int index);
		void on_AutoDetect__released ();
		void handleCurrentSongChanged (const Media::AudioInfo& ai);
		void handleHideMainOptions (bool checked);
		void handleHideLikeButtons (bool checked);
	};
}
}
}

