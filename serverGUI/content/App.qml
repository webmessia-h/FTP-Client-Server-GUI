
import QtQuick 6.2
import serverGUI
import Server
// App.qml
Window {
    width: Constants.width
    height: Constants.height

    visible: true
    title: "server"

    Greeter {
        id: initialScreen
    }
}

