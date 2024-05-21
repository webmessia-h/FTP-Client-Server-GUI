// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import clientGUI
import clientGUI 1.0

import Client

Window {
    width: Constants.width
    height: Constants.height

    visible: true
    title: "client"

    Greeter {
        id: mainScreen
    }

}

