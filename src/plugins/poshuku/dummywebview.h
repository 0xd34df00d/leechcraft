/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "interfaces/poshuku/iwebview.h"
#include "ui_dummywebview.h"

namespace LC
{
namespace Poshuku
{
	class DummyWebView : public QWidget
					   , public IWebView
	{
		Q_OBJECT

		Ui::DummyWebView Ui_;

		QAction * const DummyAction_;
	public:
		DummyWebView ();

		void SurroundingsInitialized () override;
		QWidget* GetQWidget () override;
		QList<QAction*> GetActions (ActionArea) const override;
		QAction* GetPageAction (PageAction) const override;
		QString GetTitle () const override;
		QUrl GetUrl () const override;
		QString GetHumanReadableUrl () const override;
		QIcon GetIcon () const override;
		void Load (const QUrl&, const QString&) override;
		void SetContent (const QByteArray&, const QByteArray&, const QUrl&) override;
		void ToHtml (const std::function<void (QString)>&) const override;
		void EvaluateJS (const QString&, const std::function<void (QVariant)>&, Util::BitFlags<EvaluateJSFlag>) override;
		void AddJavaScriptObject (const QString&, QObject*) override;
		void Print (bool) override;
		QPixmap MakeFullPageSnapshot () override;
		QPoint GetScrollPosition () const override;
		void SetScrollPosition (const QPoint&) override;
		double GetZoomFactor () const override;
		void SetZoomFactor (double) override;
		QString GetDefaultTextEncoding () const override;
		void SetDefaultTextEncoding (const QString&) override;
		void InitiateFind (const QString&) override;
		QMenu* CreateStandardContextMenu () override;
		IWebViewHistory_ptr GetHistory () override;
		void SetAttribute (Attribute, bool) override;
	signals:
		void earliestViewLayout () override;
		void linkHovered (const QString&, const QString&, const QString&) override;
		void storeFormData (const PageFormsData_t&) override;
		void featurePermissionRequested (const IWebView::IFeatureSecurityOrigin_ptr&, IWebView::Feature) override;
		void zoomChanged () override;
		void closeRequested () override;
		void contextMenuRequested (const QPoint&, const ContextMenuInfo&) override;

		void loadStarted ();
		void loadProgress (int);
		void loadFinished (bool);
		void iconChanged ();
		void titleChanged (const QString&);
		void urlChanged (const QUrl&);
		void urlChanged (const QString&);
		void statusBarMessage (const QString&);
	};
}
}
