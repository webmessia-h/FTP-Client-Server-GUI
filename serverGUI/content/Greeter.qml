import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Layouts
import QtQuick.Dialogs
import serverGUI

import Server
// Greeter.qml
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
        text: qsTr("<h1>Server configuration</h1>")

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
            onTextChanged: launchButton.enabled = ipInputField.text.length > 0
                           && portInputField.text.length > 0
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
            validator: RegularExpressionValidator {
                regularExpression: /^(102[4-9]|10[3-9][0-9]|1[1-9][0-9]{2}|[2-5][0-9]{3}|6[0-4][0-9]{3}|650[0-2][0-9]|6503[0-4])$/
            }
            font.family: Constants.font.family
            onTextChanged: launchButton.enabled = ipInputField.text.length >= 4
                           && portInputField.text.length >= 4
        }

        Button {
            id: launchButton
            width: 125
            height: 48
            text: qsTr("Launch Server")
            Layout.topMargin: 4
            Layout.bottomMargin: 4
            highlighted: true
            topPadding: 12
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.fillWidth: false
            enabled: false // Initially disabled

            onClicked: {
                // Ensure that both fields are not empty
                if (ipInputField.text.length >= 4
                        && portInputField.text.length >= 4) {
                    animation.start()
                    Backend.launch(ipInputField.text,
                                   parseInt(portInputField.text))
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
            when: launchButton.checked
            PropertyChanges {
                target: launchButton
                text: qsTr("Launching...")
            }
        },
        State {
            name: "Success"
            when: Backend.launched === true
            PropertyChanges {
                target: launchButton
                text: qsTr("Success")
            }
        }
    ]

    Connections {
        target: Backend
        onLaunched: {
            launchButton.checked = false
            loader.source = "Upload.qml"
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
            Backend.accept()
        }
    }
}
