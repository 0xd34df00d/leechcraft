#include "viewreemitter.h"
#include "tabcontents.h"

namespace LeechCraft
{
	ViewReemitter::ViewReemitter (QObject *parent)
	: QObject (parent)
	{
	}

	void ViewReemitter::Connect (TabContents *tc)
	{
		QTreeView *view = tc->GetUi ().PluginsTasksTree_;
#define C(x) \
		connect (view, \
				SIGNAL (x (const QModelIndex&)), \
				this, \
				SLOT (handle_##x (const QModelIndex&)));
		C (activated);
		C (clicked);
		C (doubleClicked);
		C (entered);
		C (pressed);
#undef C
		connect (view,
				SIGNAL (viewportEntered ()),
				this,
				SLOT (handleViewportEntered ()));
	}

	void ViewReemitter::ConnectModelSpecific (TabContents *tc)
	{
		QItemSelectionModel *sel = tc->GetUi ().PluginsTasksTree_->selectionModel ();
#define C(x) \
		connect (sel, \
				SIGNAL (x (const QModelIndex&, const QModelIndex&)), \
				this, \
				SLOT (handle_##x (const QModelIndex&, const QModelIndex&)));
		C (currentChanged);
		C (currentColumnChanged);
		C (currentRowChanged);
#undef C
		connect (sel,
				SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
				this,
				SLOT (handleSelectionChanged (const QItemSelection&, const QItemSelection&)));
	}

#define D(x) \
	void ViewReemitter::handle_##x (const QModelIndex& index) \
	{ \
		emit x (index, qobject_cast<QTreeView*> (sender ())); \
	}
	D (activated);
	D (clicked);
	D (doubleClicked);
	D (entered);
	D (pressed);
#undef D

	void ViewReemitter::handleViewportEntered ()
	{
		emit viewportEntered (qobject_cast<QTreeView*> (sender ()));
	}

#define D(x) \
	void ViewReemitter::handle_##x (const QModelIndex& current, \
			const QModelIndex& previous) \
	{ \
		emit x (current, previous, qobject_cast<QTreeView*> (sender ())); \
	}
	D (currentChanged);
	D (currentColumnChanged);
	D (currentRowChanged);
#undef D

	void ViewReemitter::handleSelectionChanged (const QItemSelection& selected,
			const QItemSelection& deselected)
	{
		emit selectionChanged (selected, deselected, qobject_cast<QTreeView*> (sender ()));
	}
};

