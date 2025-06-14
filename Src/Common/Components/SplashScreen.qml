import QtQuick 2.15
import QtQuick.Controls

Item {
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
            source: "qrc:/img/bankLogo"
        }
    }

    ProgressBar {
        id: progressBar
        value: 0.6
        width: 200
        height: 20
        anchors.top: imageHolderId.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        NumberAnimation {
            target: progressBar
            property: "value"
            from: 0.0
            to: 1.0
            duration: 2000
            running: progressBar.value < 1.0

            onFinished: {
                console.log("Load Complete")
            }
        }
    }
}
