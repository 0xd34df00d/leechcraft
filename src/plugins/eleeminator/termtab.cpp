/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "termtab.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QToolButton>
#include <QShortcut>
#include <QFontDialog>
#include <QUrl>
#include <QProcessEnvironment>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QKeyEvent>
#include <QtDebug>
#include <qtermwidget.h>
#include <util/xpc/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "xmlsettingsmanager.h"
#include "processgraphbuilder.h"
#include "closedialog.h"
#include "colorschemesmanager.h"
#include "termcolorschemechooser.h"
#include "termcontextmenubuilder.h"
#include "termfontchooser.h"

namespace LC::Eleeminator
{
	namespace
	{
#ifdef Q_OS_MAC
		void FixCommandControl (QKeyEvent *ev)
		{
			auto mods = ev->modifiers ();
			const bool hasCtrl = mods & Qt::ControlModifier;
			const bool hasCmd = mods & Qt::MetaModifier;
			bool changed = false;
			if (hasCtrl != hasCmd)
			{
				if (hasCtrl)
				{
					mods |= Qt::MetaModifier;
					mods &= ~Qt::ControlModifier;
				}
				else
				{
					mods |= Qt::ControlModifier;
					mods &= ~Qt::MetaModifier;
				}
				changed = true;
			}

			auto key = ev->key ();
			if (key == Qt::Key_Control)
			{
				key = Qt::Key_Meta;
				changed = true;
			}
			else if (key == Qt::Key_Meta)
			{
				key = Qt::Key_Control;
				changed = true;
			}

			if (changed)
				*ev = QKeyEvent
				{
					ev->type (),
					key,
					mods,
					ev->text (),
					ev->isAutoRepeat (),
					static_cast<ushort> (ev->count ())
				};
		}
#endif
		void SendUrl (const QUrl& url)
		{
			const auto& entity = Util::MakeEntity (url, {}, TaskParameter::FromUserInitiated);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (entity);
		}

		Qt::KeyboardModifier GetModifier (const QString& str)
		{
			if (str == "Ctrl")
				return Qt::ControlModifier;
			else if (str == "Alt")
				return Qt::AltModifier;
			else if (str == "Shift")
				return Qt::ShiftModifier;
			else if (str == "Meta")
				return Qt::MetaModifier;
			else
				return Qt::NoModifier;
		}

		void HandleUrlActivatedInTerm (const QUrl& url)
		{
			const auto modifiers = QApplication::keyboardModifiers ();
			const auto neededMod = GetModifier (XmlSettingsManager::Instance ().property ("LinkActivationModifier").toString ());
			if ((modifiers & neededMod) == neededMod)
				SendUrl (url);
		}

		void SetEnvironment (QTermWidget& term)
		{
			auto systemEnv = QProcessEnvironment::systemEnvironment ();
			if (systemEnv.value ("TERM") != "xterm")
				systemEnv.remove ("TERM");
			if (!systemEnv.contains ("TERM"))
			{
				systemEnv.insert ("TERM", "xterm");
				term.setEnvironment (systemEnv.toStringList ());
			}
		}
	}

	TermTab::TermTab (Util::ShortcutManager *scMgr,
			const TabClassInfo& tc, const ColorSchemesManager& colorSchemes, QObject *plugin)
	: TC_ (tc)
	, ParentPlugin_ { plugin }
	, Toolbar_ { new QToolBar { tr ("Terminal toolbar") } }
	, Term_ { *new QTermWidget { false } }
	{
		auto lay = new QVBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		setLayout (lay);

		lay->addWidget (&Term_);

		Term_.setFlowControlEnabled (true);
		Term_.setFlowControlWarningEnabled (true);
		Term_.setScrollBarPosition (QTermWidget::ScrollBarRight);

#ifdef Q_OS_MAC
		connect (&Term_,
				&QTermWidget::termKeyPressed,
				&Term_,
				&FixCommandControl);
#endif

		SetEnvironment (Term_);

		Term_.startShellProgram ();

		connect (&Term_,
				&QTermWidget::finished,
				this,
				&TermTab::RemoveTab);

		connect (&Term_,
				&QTermWidget::urlActivated,
				this,
				[] (const QUrl& url, bool fromMenu)
				{
					if (fromMenu)
						SendUrl (url);
					else
						HandleUrlActivatedInTerm (url);
				});

		QTimer::singleShot (0,
				&Term_,
				qOverload<> (&QTermWidget::setFocus));

		SetupToolbar (scMgr, colorSchemes);
		SetupShortcuts (scMgr);

		SetupContextMenu (Term_);

		connect (&Term_,
				&QTermWidget::bell,
				this,
				&TermTab::HandleBell);

		auto timer = new QTimer { this };
		timer->callOnTimeout (this, &TermTab::UpdateTitle);
		timer->start (3000);

		auto& xsm = XmlSettingsManager::Instance ();
		xsm.RegisterObject ({ "FiniteHistory", "HistorySize" },
				this,
				[this, &xsm]
				{
					const bool isFinite = xsm.property ("FiniteHistory").toBool ();
					const auto linesCount = isFinite ?
							xsm.property ("HistorySize").toInt () :
							-1;
					Term_.setHistorySize (linesCount);
				});
	}

	TabClassInfo TermTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* TermTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	QToolBar* TermTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void TermTab::Remove ()
	{
		const ProcessGraphBuilder builder { Term_.getShellPID () };
		if (!builder.IsEmpty ())
		{
			CloseDialog dia { builder.CreateModel (), this };
			if (dia.exec () != QDialog::Accepted)
				return;
		}

		RemoveTab ();
	}

	void TermTab::TabMadeCurrent ()
	{
		IsTabCurrent_ = true;
	}

	void TermTab::TabLostCurrent ()
	{
		IsTabCurrent_ = false;
	}

	void TermTab::SetupToolbar (Util::ShortcutManager *manager, const ColorSchemesManager& colorSchemes)
	{
		Toolbar_->addWidget (MakeColorChooser (Term_, colorSchemes).release ());
		Toolbar_->addAction (MakeFontChooser (Term_).release ());

		Toolbar_->addSeparator ();

		const auto clearAct = Toolbar_->addAction (tr ("Clear window"));
		clearAct->setProperty ("ActionIcon", "edit-clear");
		connect (clearAct,
				&QAction::triggered,
				&Term_,
				&QTermWidget::clear);
		manager->RegisterAction ("org.LeechCraft.Eleeminator.Clear", clearAct);
	}

	void TermTab::SetupShortcuts (Util::ShortcutManager *manager)
	{
		auto copySc = new QShortcut { {}, &Term_, &Term_, &QTermWidget::copyClipboard };
		manager->RegisterShortcut ("org.LeechCraft.Eleeminator.Copy", {}, copySc);

		auto pasteSc = new QShortcut { {}, &Term_, &Term_, &QTermWidget::pasteClipboard };
		manager->RegisterShortcut ("org.LeechCraft.Eleeminator.Paste", {}, pasteSc);

		auto closeSc = new QShortcut { {}, &Term_, this, &TermTab::Remove };
		manager->RegisterShortcut ("org.LeechCraft.Eleeminator.Close", {}, closeSc);
	}

	void TermTab::UpdateTitle ()
	{
		auto cwd = Term_.workingDirectory ();
		while (cwd.endsWith ('/'))
			cwd.chop (1);

		const auto& tree = ProcessGraphBuilder { Term_.getShellPID () }.GetProcessTree ();
		const auto& processName = tree.Children_.isEmpty () ?
				tree.Command_ :
				tree.Children_.value (0).Command_;

		const auto& title = cwd.isEmpty () ?
				processName :
				(cwd.section ('/', -1, -1) + " : " + processName);
		emit changeTabName (title);
	}

	void TermTab::HandleBell (const QString&) const
	{
		auto e = Util::MakeAN ("Eleeminator", tr ("Bell in terminal."), Priority::Info,
				"org.LeechCraft.Eleeminator", AN::CatTerminal, AN::TypeTerminalBell,
				"org.LeechCraft.Eleeminator.BellEvent",
				{ "Eleeminator", tr ("Bell") });
		e.Mime_ += "+advanced";
		e.Additional_ [AN::Field::TerminalActive] = IsTabCurrent_;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void TermTab::RemoveTab ()
	{
		emit removeTab ();
		deleteLater ();
	}
}
