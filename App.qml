import QtQuick
import QtQuick.Controls

Item {
    StackView {
        id: navigator
        anchors.fill: parent
        initialItem: "qrc:/Src/Common/Components/SplashScreen.qml"
    }

    Component.onCompleted: {
        console.log("StackView Load complete")
    }
}
