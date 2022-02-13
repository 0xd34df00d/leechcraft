import QtQuick 2.3
import org.LC.common 1.0

Row {
    id: rootRow

    signal bookmarkRequested()
    signal browseInfo()

    spacing: 8

    property bool bookmarkVisible: true

    BrowseButton {
        id: browseInfoImage
        onTriggered: rootRow.browseInfo()

        height: parent.height
        width: parent.height
    }

    ActionButton {
        textTooltip: qsTr("Bookmark the artist")
        onTriggered: rootRow.bookmarkRequested()
        actionIconURL: "image://ThemeIcons/bookmark-new"
        visible: rootRow.bookmarkVisible

        height: parent.height
        width: parent.height
    }
}
