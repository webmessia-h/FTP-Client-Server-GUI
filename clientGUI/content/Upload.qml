import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
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

        ProgressBar {
            id: progressBar
            anchors.centerIn: parent
            width: 400
            height: 24
            from: 0.0
            to: 1.0
            visible: Backend.progress > 0 && Backend.progress < 100
            value: (Backend.progress / 100.0)
        }
    Text {
            id: progressText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: progressBar.bottom
            anchors.topMargin: 10
            visible: progressBar.visible
            text: qsTr("Progress: %1%").arg(Backend.progress * 100.0)
            font.pointSize: 14
            color: "#1e293b"
        }

    Text {
            id: successText
            anchors.bottom: mainWindow.verticalCenter
            anchors.bottomMargin: 24
            anchors.horizontalCenter: mainWindow.horizontalCenter
            color: "#1e293b"
            wrapMode: Text.Wrap
            text: qsTr("<h1>File succesfully saved in: \n </h1>" + "<h3>"
                       + folderDialog.selectedFolder + "/" + fileNameInputField.text + "</h3>")

            font.family: Constants.font.family
            horizontalAlignment: Text.AlignHCenter
            visible: false
        }

    Text {
        id: waitingText
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        text: qsTr("<h2>Waiting for server action...</h2>")
        visible: (Backend.fileChosen ? false : true)
    }

    Connections {
        target: Backend
        onProgressChanged: {
            progressBar.value = Backend.progress
        }
        onTransferSuccess: {
            successText.visible = true
        }
    }
}
