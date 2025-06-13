import QtQuick
import QtQuick.Controls

Window {
    width: 900
    height: 650
    visible: true
    title: qsTr("Hello World")
    color: "#A47764"

    // Color Combination Mocha Mousse, Creamy White, and Deep Taupe

    Rectangle {
        id: imageHolderId
        height: 250
        width: 250
        //color: "red"
        anchors.centerIn: parent
        color: "transparent"
        Image {
            id: logoImageId
            anchors.fill: parent
            source: "qrc:/Src/Resource/Images/EveryBank_full_logo.png"
        }
    }

    ProgressBar {
        id: progressBar
        value: 0.6
        width: 200
        height: 20
        anchors.top: imageHolderId.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

}
