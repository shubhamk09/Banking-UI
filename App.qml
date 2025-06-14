import QtQuick
import QtQuick.Controls

Item {
    StackView {
        id: navigator
        anchors.fill: parent
        initialItem: "qrc:/common/SplashScreen.qml"
    }

    Component.onCompleted: {
        console.log("App Load complete")
    }
}
