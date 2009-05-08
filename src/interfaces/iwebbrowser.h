#ifndef INTERFACES_IWEBBROWSER_H
#define INTERFACES_IWEBBROWSER_H
#include <QString>
#include <QWidget>
#include <QUrl>
#include <QtPlugin>

/** @brief Common interface for a web widget.
 *
 * A widget that is capable of showing/rendering web pages should
 * implement this interface in order to communicate with other modules
 * of LeechCraft.
 */
class IWebWidget
{
public:
	virtual ~IWebWidget () {}

	/** @brief Loads a given url.
	 *
	 * The url should be UTF8-encoded.
	 *
	 * @param[in] url The url of the resource.
	 */
	virtual void Load (const QString& url) = 0;

	/** @brief Sets the contents of the web widget to the specified
	 * html.
	 *
	 * External objects such as stylesheets or images referenced in the
	 * HTML document are located relative to the base.
	 *
	 * @param[in] html The HTML with the new contents.
	 * @param[in] base Base address for resolution of external elements.
	 */
	virtual void SetHtml (const QString& html,
			const QUrl& base = QUrl ()) = 0;

	/** @brief Returns the IWebWidget as a QWidget.
	 *
	 * @return A widget corresponding to this IWebWidget.
	 */
	virtual QWidget* Widget () = 0;
};

/** @brief Base class for plugins that provide a web browser.
 */
class IWebBrowser
{
public:
	/** @brief Opens the url in the web browser itself.
	 * 
	 * @param[in] url The URL to open.
	 */
	virtual void Open (const QString& url) = 0;

	/** @brief Returns the IWebWidget for use in another modules of
	 * LeechCraft.
	 *
	 * The ownership transfers to the caller.
	 *
	 * @return The IWebWidget.
	 */
	virtual IWebWidget* GetWidget () const = 0;

	virtual ~IWebBrowser () {}
};

Q_DECLARE_INTERFACE (IWebWidget, "org.Deviant.LeechCraft.IWebWidget/1.0");
Q_DECLARE_INTERFACE (IWebBrowser, "org.Deviant.LeechCraft.IWebBrowser/1.0");

#endif

