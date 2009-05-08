#ifndef INTERFACES_IHAVESHORTCUTS_H
#define INTERFACES_IHAVESHORTCUTS_H
#include <QtPlugin>
#include <QMultiMap>
#include <QString>
#include <QKeySequence>
#include <QIcon>
#include <QMetaType>

class QAction;

namespace LeechCraft
{
	/** Contains information about the action that could be handled by
	 * the shortcut manager.
	 */
	struct ActionInfo
	{
		/// User-visible name of the action.
		QString UserVisibleText_;
		/// Default key sequence.
		QKeySequence Default_;
		/// Icon of the action.
		QIcon Icon_;

		ActionInfo ()
		{
		}

		ActionInfo (const QString& uvt,
				QKeySequence seq,
				const QIcon& icon)
		: UserVisibleText_ (uvt)
		, Default_ (seq)
		, Icon_ (icon)
		{
		}
	};
};

Q_DECLARE_METATYPE (LeechCraft::ActionInfo);

/** @brief Proxy for requesting shortcuts from the shortcut manager in
 * the Core.
 *
 * The plugin can communicate with the shortcut manager via this proxy.
 *
 * @sa IHaveShortcuts::SetShortcutProxy().
 */
class IShortcutProxy
{
public:
	/** @brief Returns a QKeySequence for the given action.
	 *
	 * Returns a QKeySequence for the action with given id for the given
	 * object which is currently set in the shortcut manager. The id
	 * is the same as in return value of
	 * IHaveShortcuts::GetActionInfo().
	 *
	 * @param[in] object The object that should be checked.
	 * @param[in] id ID of the action.
	 * @return The key sequence for the passed action.
	 */
	virtual QKeySequence GetShortcut (const QObject *object, int id) const = 0;
	virtual ~IShortcutProxy () { }
};

class IHaveShortcuts
{
public:
	/** @brief Sets shortcut proxy for this plugin to proxy.
	 *
	 * The plugin can communicate with the core and the shortcut manager
	 * via the proxy.
	 * 
	 * @sa IShortcutProxy
	 */
	virtual void SetShortcutProxy (const IShortcutProxy *proxy) = 0;

	/** @brief Sets shortcut's sequence if it has changed.
	 *
	 * The id is the same as in the return value of GetActionInfo().
	 *
	 * @param[in] id The id of the action.
	 * @param[in] sequence The new key sequence.
	 */
	virtual void SetShortcut (int id, const QKeySequence& sequence) = 0;

	/** @brief Returns information about all the shortcuts.
	 *
	 * Returns a QMap from action id to the ActionInfo. Action id would
	 * be further used in SetShortcut and IShortcutProxy::GetShortcut(),
	 * for example.
	 *
	 * @return Shortcut IDs mapped to the corresponding ActionInfo.
	 */
	virtual QMap<int, LeechCraft::ActionInfo> GetActionInfo () const = 0;

	virtual ~IHaveShortcuts () { }
};

Q_DECLARE_INTERFACE (IShortcutProxy, "org.Deviant.LeechCraft.IShortcutProxy/1.0");
Q_DECLARE_INTERFACE (IHaveShortcuts, "org.Deviant.LeechCraft.IHaveShortcuts/1.0");

#endif

