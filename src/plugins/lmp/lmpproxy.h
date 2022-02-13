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

namespace LC
{
namespace LMP
{
	class PlayerTab;

	class LMPUtilProxy : public QObject
					   , public ILMPUtilProxy
	{
		Q_OBJECT
		Q_INTERFACES (LC::LMP::ILMPUtilProxy)
	public:
		QString FindAlbumArt (const QString&, bool) const;
		QList<QFileInfo> RecIterateInfo (const QString&, bool, std::atomic<bool>*) const;
	};

	class LMPGuiProxy : public QObject
					  , public ILMPGuiProxy
	{
		Q_OBJECT
		Q_INTERFACES (LC::LMP::ILMPGuiProxy)

		PlayerTab *PlayerTab_ = nullptr;
	public:
		void SetPlayerTab (PlayerTab*);

		void AddCurrentSongTab (const QString&, QWidget*) const;
		void AddToolbarAction (QAction*) const;
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

		ILocalCollection* GetLocalCollection () const;
		ITagResolver* GetTagResolver () const;
		const ILMPUtilProxy* GetUtilProxy () const;
		const ILMPGuiProxy* GetGuiProxy () const;
		LMPGuiProxy* GetGuiProxy ();
	};
}
}
