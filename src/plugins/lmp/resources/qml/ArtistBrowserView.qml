import QtQuick 1.0
import "."

Rectangle {
    id: rootRect
    anchors.fill: parent

    signal linkActivated(string id)
    signal albumPreviewRequested(int idx)

    signal bookmarkArtistRequested(string id, string page, string tags)
    signal previewRequested(string artist)

    BioView {
        id: bioView

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width * 2 / 3

        onLinkActivated: rootRect.linkActivated(id)
        onAlbumPreviewRequested: rootRect.albumPreviewRequested(idx)
    }

    /*
    SimilarView {
        anchors.top: parent.top
        anchors.left: bioView.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
    */
}
