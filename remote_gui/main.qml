/*
 * Raspberry Pi PIC Programmer using GPIO connector
 * https://github.com/WallaceIT/picberry
 * Copyright 2016 Francesco Valla
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.1

ApplicationWindow {
    id: window
    visible: true
    title: "Picberry Remote Programmer"

    width: 500
    height: 420

    Connections {
        target: picberry
        onSendProgress: {
            progressBar.value = percentage/100.0
        }
        onSendFinish: {
            progressBar.value = 0.0
            progressBar.indeterminate = false
            content.enableButtons()
            statusLabel.text += "DONE"
        }
        onSendFinishFileSend: {
            progressBar.indeterminate = false
            statusLabel.text = "Writing memory..."
        }
        onSendFinishFirstRun: {
            statusLabel.text = "Verifying memory..."
        }
        onSendStartFileReceive: {
            statusLabel.text = "Transferring file..."
            progressBar.indeterminate = true
        }
        onSendError: {
            statusLabel.text += "ERROR!"
        }
    }

    MessageDialog {
        id: aboutDialog
        icon: StandardIcon.Information
        title: "About"
        text: "Picberry Remote Programmer"
        informativeText: "Remote PIC Programmer, "
                          + "to be used with a picberry daemon."
                          + "\n\n"
                          + "c 2016 Francesco Valla"
                          + "\n\n"
                          + "https://github.com/WallaceIT/picberry"
    }

    FileDialog {
        id: saveDialog
        title: "Save as..."
        selectExisting: false
        nameFilters: ["HEX File (*.hex)", "All files (*)"]
        onAccepted: {
            content.disableButtons()
            statusLabel.text = "Reading memory..."
            picberry.readMemory(saveDialog.fileUrl)
        }
    }

    FileDialog {
        id: openDialog
        title: "Save as..."
        selectExisting: true
        selectMultiple: false
        nameFilters: ["HEX File (*.hex)"]
        onAccepted: {
            content.disableButtons()
            statusLabel.text = "Transferring file..."
            progressBar.indeterminate = true
            picberry.writeMemory(openDialog.fileUrl)
        }
    }

    Action {
        id: copyAction
        text: "&Copy"
        shortcut: StandardKey.Copy
        iconName: "edit-copy"
        enabled: (!!activeFocusItem && !!activeFocusItem["copy"])
        onTriggered: activeFocusItem.copy()
    }

    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem {
                text: "E&xit"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: "&Edit"
            //MenuItem { action: cutAction }
        }
        Menu {
            title: "&Help"
            MenuItem {
                text: "About..."
                onTriggered: aboutDialog.open()
            }
        }
    }

    toolBar: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: spacing
            Item { Layout.fillWidth: true }
            Label { text: "Host " }
            TextField {
                id: addressField
                maximumLength: 15
                //inputMask: "000.000.000.000"
                text: "192.168.1.72"
            }
            Label { text: ":" }
            TextField {
                id: portField
                text: "11567"
                maximumLength: 5
                //inputMask: "00000"
            }
            Button {
                id: connectButton
                Layout.fillWidth: true
                text: "Connect"
                onClicked: {
                    var result = picberry.connectSocket(addressField.text, portField.text)
                    if(result){
                        content.enabled = true
                        this.enabled = false
                        disconnectButton.enabled = true
                        var picberry_version = picberry.getPicberryVersion();
                        statusLabel.text = "Connected to picberry v" + picberry_version
                    }
                }
            }
            Button {
                id: disconnectButton
                Layout.fillWidth: true
                enabled: false
                text: "Disconnect"
                onClicked: {
                    if(programModeSwitch.checked){
                        picberry.exitProgramMode()
                        programModeSwitch.checked = false
                    }
                    picberry.disconnectSocket()
                    content.enabled = false
                    this.enabled = false
                    connectButton.enabled = true
                    statusLabel.text = "Not connected"
                }
            }
        }
    }

    Item {
        id: content
        enabled: false
        anchors.fill: parent

        function disableButtons() {
            disconnectButton.enabled = false
            programModeSwitch.enabled = false
            readButton.enabled = false
            writeButton.enabled = false
            eraseButton.enabled = false
        }

        function enableButtons() {
            disconnectButton.enabled = true
            programModeSwitch.enabled = true
            readButton.enabled = true
            writeButton.enabled = true
            eraseButton.enabled = true
        }

        ColumnLayout {

            Layout.fillWidth: true
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: spacing
            anchors.rightMargin: spacing
            anchors.topMargin: spacing

            GroupBox {
                title: "Device"
                Layout.fillWidth: true
                ColumnLayout{
                    anchors.fill: parent
                    RowLayout {
                        Layout.fillWidth: true
                        Label {text: "Family: "}
                        ComboBox {
                            id: familyListBox
                            Layout.fillWidth: true
                            model: ListModel {
                                id: familyList
                                ListElement { text: "dspic33e"; value: "0" }
                                ListElement { text: "dspic33f"; value: "1" }
                                ListElement { text: "pic18fj"; value: "2" }
                                ListElement { text: "pic24fj"; value: "3" }
                            }
                        }
                        Item {width: 20}
                        Label {text: "Program Mode: "}
                        Switch {
                            id: programModeSwitch
                            checked: false
                            onCheckedChanged: {
                                if(this.checked){
                                    picberry.enterProgramMode()
                                    picberry.setFamily(familyList.get(familyListBox.currentIndex).value)
                                    familyListBox.enabled = false
                                    var d = eval('new Object(' + picberry.getDeviceID() + ')')
                                    deviceName.text = d.DevName
                                    deviceID.text = d.DevID
                                    deviceRevision.text = d.DevRev
                                }
                                else{
                                    picberry.exitProgramMode()
                                    familyListBox.enabled = true
                                    deviceName.text = "N.A."
                                    deviceID.text = "0x0000"
                                    deviceRevision.text = "0x0000"
                                }
                            }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label {text: "Device name: "}
                        Label {id: deviceName; text: "N.A.";}
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label {text: "Device ID: "}
                        Label {id: deviceID; text: "0x0000"}
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label {text: "Revision: "}
                        Label {id: deviceRevision; text: "0x0000"}
                    }
                }
            }

            GroupBox {
                title: "Actions"
                enabled: programModeSwitch.checked
                Layout.fillWidth: true
                ColumnLayout{
                    anchors.fill: parent
                    Button {
                        id: readButton
                        Layout.fillWidth: true
                        text: "Read"
                        onClicked: {
                            saveDialog.open()
                        }
                    }
                    Button {
                        id: writeButton
                        Layout.fillWidth: true
                        text: "Write"
                        onClicked: {
                            openDialog.open()
                        }
                    }
                    Button {
                        id: eraseButton
                        Layout.fillWidth: true
                        text: "Erase"
                        onClicked: {
                            content.disableButtons()
                            statusLabel.text = "Erasing memory..."
                            progressBar.indeterminate = true
                            picberry.eraseMemory()
                        }
                    }
                    ProgressBar {
                        id: progressBar
                        value: 0
                        Layout.fillWidth: true
                    }
                }
            }

            Button {
                id: resetButton
                Layout.fillWidth: true
                text: "Reset"
                enabled: !programModeSwitch.checked
            }

        }
    }

    statusBar: StatusBar {
        Layout.fillWidth: true
        RowLayout {
            anchors.fill: parent
            Label { id: statusLabel; text: "Not connected" }
        }
    }
}
