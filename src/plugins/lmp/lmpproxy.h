/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/lmp/ilmpproxy.h"
#include "interfaces/lmp/ilmputilproxy.h"
#include "interfaces/lmp/ilmpguiproxy.h"

namespace LC::LMP
{
	class PlayerTab;

	class LMPUtilProxy : public QObject
					   , public ILMPUtilProxy
	{
		Q_OBJECT
		Q_INTERFACES (LC::LMP::ILMPUtilProxy)
	public:
		QString FindAlbumArt (const QString&, bool) const override;
		QList<QFileInfo> RecIterateInfo (const QString&, bool, std::atomic<bool>*) const override;
	};

	class LMPGuiProxy : public QObject
					  , public ILMPGuiProxy
	{
		Q_OBJECT
		Q_INTERFACES (LC::LMP::ILMPGuiProxy)

		PlayerTab *PlayerTab_ = nullptr;
	public:
		void SetPlayerTab (PlayerTab*);

		void AddCurrentSongTab (const QString&, QWidget*) const override;
		void AddToolbarAction (QAction*) const override;
	};

	class LMPProxy : public QObject
				   , public ILMPProxy
	{
		Q_OBJECT
		Q_INTERFACES (LC::LMP::ILMPProxy)

		ILocalCollection * const LocalCollection_;
		ITagResolver * const TagResolver_;

		LMPUtilProxy UtilProxy_;
		LMPGuiProxy GuiProxy_;
	public:
		LMPProxy (ILocalCollection*, ITagResolver*);

		ILocalCollection* GetLocalCollection () const override;
		ITagResolver* GetTagResolver () const override;
		const ILMPUtilProxy* GetUtilProxy () const override;
		const ILMPGuiProxy* GetGuiProxy () const override;

		LMPGuiProxy* GetGuiProxy ();
	};
}
