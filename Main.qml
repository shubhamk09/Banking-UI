import QtQuick

Window {
    width: 900
    height: 650
    visible: true
    title: qsTr("Hello World")
    color: "#A47764"

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
}
