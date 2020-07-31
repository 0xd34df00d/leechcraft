/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dummywebview.h"
#include <QIcon>
#include <QAction>

namespace LC
{
namespace Poshuku
{
	DummyWebView::DummyWebView ()
	: DummyAction_ { new QAction { this } }
	{
		Ui_.setupUi (this);
	}

	void DummyWebView::SurroundingsInitialized ()
	{
	}

	QWidget* DummyWebView::GetQWidget ()
	{
		return this;
	}

	QList<QAction*> DummyWebView::GetActions (IWebView::ActionArea) const
	{
		return {};
	}

	QAction* DummyWebView::GetPageAction (IWebView::PageAction) const
	{
		return DummyAction_;
	}

	QString DummyWebView::GetTitle () const
	{
		return {};
	}

	QUrl DummyWebView::GetUrl () const
	{
		return {};
	}

	QString DummyWebView::GetHumanReadableUrl () const
	{
		return {};
	}

	QIcon DummyWebView::GetIcon () const
	{
		return {};
	}

	void DummyWebView::Load (const QUrl&, const QString&)
	{
	}

	void DummyWebView::SetContent (const QByteArray&, const QByteArray&, const QUrl&)
	{
	}

	void DummyWebView::ToHtml (const std::function<void (QString)>& function) const
	{
		function ({});
	}

	void DummyWebView::EvaluateJS (const QString&,
			const std::function<void (QVariant)>&,
			Util::BitFlags<IWebView::EvaluateJSFlag>)
	{
	}

	void DummyWebView::AddJavaScriptObject (const QString&, QObject*)
	{
	}

	void DummyWebView::Print (bool)
	{
	}

	QPixmap DummyWebView::MakeFullPageSnapshot ()
	{
		return {};
	}

	QPoint DummyWebView::GetScrollPosition () const
	{
		return {};
	}

	void DummyWebView::SetScrollPosition (const QPoint&)
	{
	}

	double DummyWebView::GetZoomFactor () const
	{
		return 1;
	}

	void DummyWebView::SetZoomFactor (double)
	{
	}

	QString DummyWebView::GetDefaultTextEncoding () const
	{
		return {};
	}

	void DummyWebView::SetDefaultTextEncoding (const QString&)
	{
	}

	void DummyWebView::InitiateFind (const QString&)
	{
	}

	QMenu* DummyWebView::CreateStandardContextMenu ()
	{
		return nullptr;
	}

	IWebViewHistory_ptr DummyWebView::GetHistory ()
	{
		return {};
	}

	void DummyWebView::SetAttribute (IWebView::Attribute, bool)
	{
	}
}
}
