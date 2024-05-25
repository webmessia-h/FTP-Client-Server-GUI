import QtQuick
import QtQuick.Layouts
import Client
import clientGUI 1.0

Item {
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.left: parent.left
    anchors.top: parent.top
    // Display current connection status and port
    Connections {
        target: Backend

        onIsConnectedChanged: {
            status.text = "<h3>Connection Status: "
                    + (Backend.connected ? "Connected</h3>" : "Disconnected</h3>")
        }

        onPortChanged: {
            port.text = "<h3>Port: " + Backend.port + "</h3>"
        }
    }
    Text {
        id: status
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: 12
            leftMargin: 20
        }
        text: "<h3>Connection Status: "
                    + (Backend.connected ? "Connected</h3>" : "Disconnected</h3>")
    }

    Text {
        id: port
        anchors {
            top: parent.top
            left: parent.left
            leftMargin: 6
            topMargin: 10
        }
        text: "<h3>Port: " + Backend.port + "</h3>"
    }
    Text {
            anchors {
                top: parent.top
                right: parent.right
                rightMargin: 6
                topMargin: 10
            }
            text: "<h3>Ip: " + Backend.ip + "</h3>"

        }
}
