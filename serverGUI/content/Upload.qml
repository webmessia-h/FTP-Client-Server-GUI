import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import serverGUI

import Server
import listModel
// Upload.qml

Rectangle {

    id: mainWindow
    width: Constants.width
    height: Constants.height
    color: Constants.backgroundColor

    StatusBar {
        id: statusBar
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
        id: waitingText
        anchors.centerIn: parent
        color: "#1e293b"
        text: qsTr("<h1>Waiting for client to connect...</h1>")

        font.family: Constants.font.family
        horizontalAlignment: Text.AlignHCenter
        visible: true
    }

    // File list dialog
    Dialog {
        id: fileListDialog
        title: "Select a File"
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 600
        height: 400

        ColumnLayout {
            spacing: 4
            anchors.fill: parent

            ListView {
                id: fileListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: Backend.fileListModel
                clip: true

                delegate: Item {
                    width: fileListView.width
                    height: 40

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (fileListView) {
                                fileListView.currentIndex = index
                                fileListDialog.selectedItem = model.string
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: fileListView.currentIndex === index ? "lightblue" : "white"

                            Text {
                                text: model.string
                                anchors.centerIn: parent
                                wrapMode: Text.Wrap
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }
        }
        property string selectedItem: ""
        onAccepted: {
            if (selectedItem.length > 0) {
                fileListDialog.close()
                refreshButton.visible = false
                listButton.visible = false
                folderDialog.open()
            } else {
                console.error("No item selected")
            }
        }
    }

    Button {
        id: refreshButton
        visible: false
        anchors.bottom: mainWindow.bottom
        anchors.horizontalCenter: mainWindow.horizontalCenter
        anchors.bottomMargin: 52
        text: "Refresh"
        onClicked: {
            Backend.receive_file_list()
        }
    }

    Button {
        id: listButton
        visible: false
        anchors.bottom: mainWindow.bottom
        anchors.horizontalCenter: mainWindow.horizontalCenter
        anchors.bottomMargin: 96
        text: "Open file List"
        onClicked: {
            fileListDialog.open()
            waitingText.visible = false
        }
    }

    // Save path dialog
    FolderDialog {
        id: folderDialog
        title: "Select a folder to save file"
        acceptLabel: "Confirm"
        currentFolder: StandardPaths.writableLocation(
                           StandardPaths.HomeLocation)
        onAccepted: {
            fileNameInputField.visible = true
            saveButton.visible = true
        }
    }

    TextField {
        visible: false
        id: fileNameInputField
        width: 210
        height: 34
        anchors.centerIn: parent ////////////////
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.fillHeight: false
        Layout.fillWidth: true
        placeholderText: "File name and extension:"
        font.family: Constants.font.family
        validator: RegularExpressionValidator {
            regularExpression: /^[a-zA-Z0-9._-]+\.[a-zA-Z0-9]+$/
        }
        Layout.bottomMargin: 2
        onTextChanged: saveButton.enabled = fileNameInputField.text.length > 0
    }

    Button {
        id: saveButton
        visible: false
        width: 125
        height: 48
        text: qsTr("Confirm")
        anchors.bottom: mainWindow.bottom
        anchors.horizontalCenter: mainWindow.horizontalCenter
        anchors.bottomMargin: 52
        highlighted: true
        topPadding: 12
        enabled: false // Initially disabled

        onClicked: {
            if (fileNameInputField.text.length > 0) {
                Backend.request_upload(fileListDialog.selectedItem,
                            folderDialog.selectedFolder + "/" + fileNameInputField.text)
                fileNameInputField.visible = false
                saveButton.visible = false
                progressBar.visible = true
            }
        }
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

    Connections {
        target: Backend
        onIsConnectedChanged: {
            if (Backend.connected === true) {
                waitingText.text = qsTr("<h1>Ready to receive file list</h1>")
                refreshButton.visible = true
            }
        }
        onFileListReceived: {
            fileListView.model = Backend.fileListModel // just in case
            waitingText.text = qsTr("<h1>File list received!</h1>")
            listButton.visible = true
        }
        onProgressChanged: {
            progressBar.value = Backend.progress
        }
        onTransferSuccess: {
            successText.visible = true
        }
    }
}
