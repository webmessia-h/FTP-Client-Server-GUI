import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import clientGUI
import Client

Rectangle {
    id: mainWindow
    width: Constants.width
    height: Constants.height
    color: Constants.backgroundColor
    Text {
        id: greetText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 64
        color: "#1e293b"
        text: qsTr("<h1>Server Access Configuration</h1>")

        font.family: Constants.font.family
        horizontalAlignment: Text.AlignHCenter
    }
    ColumnLayout {
        id: columnLayout
        width: 210
        height: 140
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: greetText.bottom
        anchors.topMargin: 96
        spacing: 2

        TextField {
            id: ipInputField
            width: 210
            Layout.preferredHeight: 34
            Layout.topMargin: 12
            bottomPadding: 4
            leftPadding: 12
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillHeight: false
            Layout.fillWidth: true
            placeholderText: "IP-address:"
            font.family: Constants.font.family
            validator: RegularExpressionValidator {
                regularExpression: /\A(?:\d{1,3}\.){3}\d{1,3}\z/
            }
            Layout.bottomMargin: 2
            onTextChanged: connectButton.enabled = ipInputField.text.length >= 4
                           && portInputField.text.length >= 4
        }

        TextField {
            id: portInputField
            width: 210
            Layout.preferredHeight: 34
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            leftPadding: 12
            Layout.topMargin: 2
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            placeholderText: "Port:"
            validator: IntValidator {
                bottom: 1025
                top: 65535
            }
            font.family: Constants.font.family
            onTextChanged: connectButton.enabled = ipInputField.text.length >= 4
                           && portInputField.text.length >= 4
        }

        Button {
            id: connectButton
            width: 125
            height: 48
            text: qsTr("Connect to server")
            Layout.topMargin: 4
            Layout.bottomMargin: 4
            highlighted: true
            topPadding: 12
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.fillWidth: false
            enabled: false
            Connections {
                target: connectButton
                onClicked: {
                    if (ipInputField.text.length >= 4
                            && portInputField.text.length >= 4) {
                        animation.start()
                        Backend.connect(ipInputField.text,
                                        parseInt(portInputField.text))
                    }
                }
            }
        }
    }

    SequentialAnimation {
        id: animation

        ColorAnimation {
            id: colorAnimation1
            target: mainWindow
            property: "color"
            to: "#ccd0da"
            from: Constants.backgroundColor
            duration: 600
        }

        ColorAnimation {
            id: colorAnimation2
            target: mainWindow
            property: "color"
            to: Constants.backgroundColor
            from: "#ccd0da"
            duration: 600
        }
    }

    states: [
        State {
            name: "clicked"
            when: connectButton.checked
            PropertyChanges {
                target: connectButton
                text: qsTr("Connecting...")
            }
        },
        State {
            name: "connected"
            when: Backend.connected === true
            PropertyChanges {
                target: connectButton
                text: qsTr("Connected")
            }
        }
    ]
    Connections {
        target: Backend
        onConnected: {
            connectButton.checked = false
            connectButton.state = (Backend.connected ? "Connected</h3>" : "Failed to connect</h3>")
            loader.source = "ChooseDir.qml"
        }
    }
    Loader {
        id: loader
        anchors.fill: parent
        source: ""
    }
}
