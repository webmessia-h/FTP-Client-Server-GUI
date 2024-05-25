import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import clientGUI 1.0

import Client

Rectangle {
    id: mainWindow
    width: Constants.width
    height: Constants.height
    color: Constants.backgroundColor

    StatusBar {
        visible: true
    }

    MenuItem {
        anchors.centerIn: parent
        Button {
            text: qsTr("Choose folder")
            onClicked: folderDialog.open()
        }
    }

    FolderDialog {
        id: folderDialog
        title: "Select a folder"
        acceptLabel: "Confirm"
        currentFolder: StandardPaths.writableLocation(
                           StandardPaths.HomeLocation)
        onAccepted: {
            folderDialog.close()
            loader.source = "Upload.qml"
            Backend.send_file_list(folderDialog.selectedFolder)
        }
      }

       Loader {
        id: loader
        anchors.fill: parent
        source: ""
        onLoaded: {
            startAcceptTimer.start()
        }
    }

    Timer {
        id: startAcceptTimer
        interval: 2000
        repeat: false
        onTriggered: {
            Backend.handle_request()
        }
    }

}
