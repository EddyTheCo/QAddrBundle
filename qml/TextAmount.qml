import QtQuick
import QtQuick.Controls
import Esterv.Iota.AddrBundle
import Esterv.Styles.Simple
Text
{
    id: control
    required property Qml64 amount;
    property bool change:true
    color:Style.frontColor1
    text: (Object.keys(amount.json).length === 0)?'':((!amount.json.default&&change)?(amount.json.shortValue.value+ '<font color=\"'+Style.frontColor2+'\">'+amount.json.shortValue.unit+'</font>'):
                                   amount.json.largeValue.value+ '<font color=\"'+Style.frontColor2+'\">'+amount.json.largeValue.unit+'</font>');

    MouseArea {
        anchors.fill: parent
        onClicked:
        {
            control.change=!control.change;
        }
    }
}
