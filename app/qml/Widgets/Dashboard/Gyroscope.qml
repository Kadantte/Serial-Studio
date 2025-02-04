/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  //
  // Widget data inputs
  //
  property color color
  property GyroscopeModel model: GyroscopeModel {}

  //
  // Custom properties
  //
  property real yawAngle: root.model.yaw
  property real rollAngle: root.model.roll
  property real pitchAngle: root.model.pitch

  //
  // Animations
  //
  Behavior on yawAngle {NumberAnimation{}}
  Behavior on rollAngle {NumberAnimation{}}
  Behavior on pitchAngle {NumberAnimation{}}

  //
  // Artificial horizon
  //
  Item {
    id: artificialHorizon

    antialiasing: true
    anchors.fill: parent
    anchors.margins: -root.height / 2

    Rectangle {
      id: sky
      antialiasing: true
      anchors.fill: parent
      anchors.topMargin: -1 * parent.height
      gradient: Gradient {
        GradientStop {
          position: 0
          color: "#5759A6"
        }

        GradientStop {
          position: 1
          color: "#7A7BBB"
        }
      }
    }

    Rectangle {
      antialiasing: true
      height: parent.height * 1.5
      anchors {
        left: sky.left
        right: sky.right
        bottom: sky.bottom
        bottomMargin: -parent.height
      }

      gradient: Gradient {
        GradientStop {
          position: 1
          color: "#672122"
        }

        GradientStop {
          position: 0
          color: "#A63732"
        }
      }

      Rectangle {
        height: 2
        color: "#fff"
        antialiasing: true
        anchors {
          top: parent.top
          left: parent.left
          right: parent.right
        }
      }
    }

    transform: [
      Translate {
        y: root.pitchAngle * root.height / 45
      },
      Rotation {
        angle: -root.rollAngle
        origin.x: artificialHorizon.width  / 2
        origin.y: artificialHorizon.height / 2
      }
    ]
  }

  //
  // Pitch reticles
  //
  Item {
    id: pitchIndicator

    antialiasing: true
    height: root.height - 4 - angles.height

    anchors {
      left: parent.left
      right: parent.right
    }

    readonly property var reticleHeight: Math.max(16, pitchIndicator.height / 20)

    Item {
      antialiasing: true
      width: parent.width
      height: parent.height

      Column {
        id: column
        spacing: -4
        antialiasing: true
        anchors.centerIn: parent

        Repeater {
          model: 37
          delegate: Item {
            id: reticle
            antialiasing: true
            width: pitchIndicator.width
            height: pitchIndicator.reticleHeight

            opacity: {
              var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
              var distance = Math.abs(reticleY - rollDial.height / 2);
              var fadeDistance = rollDial.height / 2;
              return Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
            }

            onVisibleChanged: {
              var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
              var distance = Math.abs(reticleY - rollDial.height / 2);
              var fadeDistance = rollDial.height / 2;
              opacity = Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
            }

            onYChanged: {
              var reticleY = reticle.mapToItem(rollDial, 0, 0).y;
              var distance = Math.abs(reticleY - rollDial.height / 2);
              var fadeDistance = rollDial.height / 2;
              opacity = Math.max(0, Math.min(1, (fadeDistance - distance) / fadeDistance));
            }

            visible: Math.abs(pitch - root.pitchAngle) <= 20
            readonly property int pitch: -(index * 5 - 90)

            Rectangle {
              id: _line
              height: 2
              color: "#fff"
              antialiasing: true
              anchors.centerIn: parent
              opacity: (reticle.pitch % 10) === 0 ? 1 : 0.8
              width: (reticle.pitch % 10) === 0 ? pitchIndicator.width / 4 :
                                                  pitchIndicator.width / 8
            }

            Label {
              id: leftLabel
              color: "#fff"
              antialiasing: true
              text: reticle.pitch
              anchors.centerIn: parent
              font: Cpp_Misc_CommonFonts.monoFont
              opacity: root.height >= 120 ? 1 : 0
              visible: (reticle.pitch != 0) && (reticle.pitch % 10) === 0
              anchors.horizontalCenterOffset: - 1 * (_line.width + 32) / 2
            }

            Label {
              color: "#fff"
              antialiasing: true
              text: reticle.pitch
              anchors.centerIn: parent
              font: Cpp_Misc_CommonFonts.monoFont
              opacity: root.height >= 120 ? 1 : 0
              anchors.horizontalCenterOffset: (_line.width + 32) / 2
              visible: (reticle.pitch != 0) && (reticle.pitch % 10) === 0
            }
          }
        }
      }

      transform: [Translate {
          y: (root.pitchAngle * pitchIndicator.reticleHeight / 5) + (pitchIndicator.reticleHeight / 2)
        }
      ]
    }

    transform: [
      Rotation {
        angle: -root.rollAngle
        origin.x: root.width / 2
        origin.y: root.height / 2
      }
    ]
  }

  //
  // Angles indicator
  //
  Item {
    id: angles
    visible: root.height >= 120

    width: parent.width
    height: visible ? 24 : 0

    anchors {
      bottomMargin: 4
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }

    RowLayout {
      spacing: 4
      anchors.fill: parent

      Item {
        implicitWidth: 4
      }

      Rectangle {
        color: "#333"
        border.width: 1
        implicitHeight: 24
        border.color: "#fff"
        Layout.fillWidth: true

        Text {
          color: "#ffffff"
          elide: Qt.ElideLeft
          anchors.fill: parent
          verticalAlignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.monoFont
          horizontalAlignment: Qt.AlignHCenter
          text: qsTr("Roll: %1").arg(root.rollAngle.toFixed(2) + "°")
        }
      }

      Rectangle {
        color: "#333"
        border.width: 1
        implicitHeight: 24
        border.color: "#fff"
        Layout.fillWidth: true

        Text {
          color: "#ffffff"
          elide: Qt.ElideLeft
          anchors.fill: parent
          verticalAlignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.monoFont
          horizontalAlignment: Qt.AlignHCenter
          text: qsTr("Yaw: %1").arg(root.yawAngle.toFixed(2) + "°")
        }
      }

      Rectangle {
        color: "#333"
        border.width: 1
        implicitHeight: 24
        border.color: "#fff"
        Layout.fillWidth: true

        Text {
          color: "#ffffff"
          elide: Qt.ElideLeft
          anchors.fill: parent
          verticalAlignment: Qt.AlignVCenter
          font: Cpp_Misc_CommonFonts.monoFont
          horizontalAlignment: Qt.AlignHCenter
          text: qsTr("Pitch: %1").arg(root.pitchAngle.toFixed(2) + "°")
        }
      }

      Item {
        implicitWidth: 4
      }
    }
  }

  //
  // Roll dial visual effects
  //
  MultiEffect {
    blur: 1
    blurMax: 64
    brightness: 0.6
    saturation: 0.1
    blurEnabled: true
    source: rollDial
    anchors.fill: rollDial
  }

  //
  // Roll pointer visual effects
  //
  MultiEffect {
    blur: 1
    blurMax: 64
    brightness: 0.6
    saturation: 0.1
    blurEnabled: true
    source: rollPointer
    anchors.fill: rollPointer
  }

  //
  // Cross hair visual effects
  //
  MultiEffect {
    blur: 1
    blurMax: 64
    brightness: 0.6
    saturation: 0.1
    blurEnabled: true
    source: crosshair
    anchors.fill: crosshair
  }


  //
  // Roll dial
  //
  Image {
    id: rollDial
    anchors.fill: parent
    sourceSize.height: parent.height
    fillMode: Image.PreserveAspectFit
    source: "qrc:/rcc/instruments/attitude_dial.svg"

    transform: Rotation {
      angle: -root.rollAngle
      origin.x: root.width / 2
      origin.y: root.height / 2
    }
  }

  //
  // Roll pointer
  //
  Image {
    id: rollPointer
    anchors.fill: parent
    sourceSize.height: parent.height
    fillMode: Image.PreserveAspectFit
    source: "qrc:/rcc/instruments/attitude_pointer.svg"
  }

  //
  // Crosshair
  //
  Image {
    id: crosshair
    sourceSize.width: width
    anchors.centerIn: parent
    width: parent.width * 0.75
    fillMode: Image.PreserveAspectFit
    source: "qrc:/rcc/instruments/attitude_crosshair.svg"
  }
}
