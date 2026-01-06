
////////////////////////
/// Import libraries ///
////////////////////////
import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning



Item {
    id: window

    ////////////////////////
    /// Global variables ///
    ////////////////////////
    //Position
    property double mapCenterLatitude:  35
    property double mapCenterLongitude: 0

    //Cursor
    property double cursorLatitude: NaN
    property double cursorLongitude: NaN
    property double cursorDistanceBoat: NaN
    property double cursorBearingBoat: NaN

    //Boat data
    property string noData: "No Data"
    property string boatDate: ""
    property string boatTime: ""
    property double boatLatitude: 0
    property double boatLongitude: 0
    property double boatHeading: 0          //°
    property double boatCourse: 0           //°
    property double boatDepth: 0            //meters
    property double boatSpeed: 0            //knots
    property double boatWaterTemperature: 0 //°C

    //Boat data received check
    property bool boatDateReceived: false
    property bool boatTimeReceived: false
    property bool boatPositionReceived: false
    property bool boatPositionInit: false
    property bool boatHeadingReceived: false
    property bool boatCourseReceived: false
    property bool boatDepthReceived: false
    property bool boatSpeedReceived: false
    property bool boatWaterTemperatureReceived: false

    //Labels
    property int labelRightSideWidth: 135
    property int labelLeftSideWidth: 135
    property int labelPadding: 8
    property int labelLateralMargin: 8
    property int labelVerticalMargin: 8
    property int labelFontSize: 14
    property int labelBackgroundRadius: 4
    property string labelColor: "white"
    property string labelBackgroundColor: "grey"

    //Zoom
    property double mapZoomLevel: 3
    property double zoomSpeed: 0.2
    property double lastWheelRotation: 0

    //Markers
    property Component redMarker: redMarkerImg
    property Component boatMarker: boatMarkerImg
    property int userMarkerCount: 0

    // Declare boat marker reference
    property var boatMarkerRef: null

    //Right-click Menu
    property int rightClickMenuWidth: 150

    //Map
    property bool showUI: true
    property bool followBoat: false
    property string followBoatText: followBoat ? "Unfollow Boat" : "Follow Boat"

    // View modes
    property int mapViewMode: 0
    property real mapRotation: {
        switch (mapViewMode)
        {
            case 0: default: return 0   // North Up
            case 1: return boatHeading  // Heading Up
            case 2: return boatCourse   // COG Up
        }
    }

    //Timer Data Update
    property int timeBeforePositionLost: 10
    property int timeBeforeGeneralDataLost: 5
    property string textTimerPositionUpdate: "No Position Data"

    property double timeLastUtcDate: 0
    property double timeLastUtcTime: 0
    property double timeLastPosition: 0
    property double timeLastHeading: 0
    property double timeLastCourse: 0
    property double timeLastSpeed: 0
    property double timeLastDepth: 0
    property double timeLastWaterTemp: 0
    property double elapsedSec: 0

    //Heading & COG lines
    property int boatLinesDistance: 155*boatSpeed // ~5min trip



    //////////////////
    /// Map Plugin ///
    //////////////////
    Plugin {
        id: osmPlugin
        name: "osm"

        PluginParameter {
            name:"osm.mapping.custom.host"
            value: "https://tile.openstreetmap.org/"
        }

        //For offline tiles (optionnal)
        PluginParameter {
           name: "osm.mapping.offline.directory"
           value: "/path/to/tiles"
        }

        //Disable Qt default provider
        PluginParameter {
           name: "osm.mapping.providersrepository.disabled"
           value: true
        }

       //Custom tile server (OpenStreetMap standard)
        PluginParameter {
           name: "osm.mapping.host"
           value: "https://tile.openstreetmap.org/"
        }

        //For high-DPI tiles on high-resolution displays (optionnal)
        PluginParameter {
           name: "osm.mapping.highdpi_tiles"
           value: true
        }
    }

    Map {
        id: map
        anchors.fill: parent
        center: QtPositioning.coordinate(mapCenterLatitude, mapCenterLongitude)
        zoomLevel: mapZoomLevel
        activeMapType: map.supportedMapTypes[map.supportedMapTypes.length - 1]
        bearing: mapRotation

        //Load OSM plugin
        plugin: osmPlugin

        //Heading line
        MapPolyline {
            visible: (boatPositionReceived && boatHeadingReceived)
            line.width: 3
            line.color: "red"

            path: [
                QtPositioning.coordinate(boatLatitude, boatLongitude),
                destinationCoordinate(boatLatitude, boatLongitude, boatHeading, boatLinesDistance)
            ]
        }

        //COG line
        MapPolyline {
            visible: (boatPositionReceived && boatCourseReceived)
            line.width: 1
            line.color: "blue"

            path: [
                QtPositioning.coordinate(boatLatitude, boatLongitude),
                destinationCoordinate(boatLatitude, boatLongitude, boatCourse, boatLinesDistance)
            ]
        }
    }



    ////////////////
    /// Connects ///
    ////////////////
    //Update mouse position on boat movement
    Connections {
        function onBoatLatitudeChanged() {
            updateCursorCalculations()
        }

        function onBoatLongitudeChanged() {
            updateCursorCalculations()
        }
    }



    ////////////
    /// Zoom ///
    ////////////
    WheelHandler {
        id: wheelZoomHandler
        target: map

        onWheel: function(event) {
            var cursorPoint = Qt.point(event.x, event.y)
            var cursorCoordBefore = map.toCoordinate(cursorPoint)

            if (event.angleDelta.y > 0)
                map.zoomLevel = Math.min(map.maximumZoomLevel, map.zoomLevel + zoomSpeed)
            else if (event.angleDelta.y < 0)
                map.zoomLevel = Math.max(map.minimumZoomLevel, map.zoomLevel - zoomSpeed)

            var cursorCoordAfter = map.toCoordinate(cursorPoint)

            // Adjust map center so cursor stays fixed
            var latShift = cursorCoordBefore.latitude - cursorCoordAfter.latitude
            var lonShift = cursorCoordBefore.longitude - cursorCoordAfter.longitude

            map.center = QtPositioning.coordinate(map.center.latitude + latShift, map.center.longitude + lonShift)
            mapZoomLevel = map.zoomLevel
        }
    }



    //////////////////
    /// Mouse Area ///
    //////////////////
    MouseArea
    {
        id: mouseArea
        anchors.fill: map
        hoverEnabled: true
        acceptedButtons: Qt.RightButton | Qt.LeftButton

        property var lastCoord
        property bool dragging: false
        property var coordinate: map.toCoordinate(Qt.point(mouseX, mouseY))

        //Dragging map
        onPressed: function(mouse) {
            if (mouse.button === Qt.LeftButton) {
                lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                dragging = true
            }
        }

        onReleased: function(mouse) {
            dragging = false
        }

        onPositionChanged: function(mouse) {
            if (dragging && mouse.buttons === Qt.LeftButton) {
                var currentCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                var dx = lastCoord.longitude - currentCoord.longitude
                var dy = lastCoord.latitude - currentCoord.latitude
                map.center = QtPositioning.coordinate(map.center.latitude + dy, map.center.longitude + dx)
                lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
            }
            else
                lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))

            cursorLatitude = coordinate.latitude
            cursorLongitude = coordinate.longitude

            cursorDistanceBoat = haversineDistance(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
            cursorBearingBoat = calculateBearing(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
        }

        // Right-click menu
        onClicked: function(mouse){
             if (mouse.button === Qt.RightButton) {
                 contextMenu.popup()
             }
         }
    }



    ////////////////////////
    /// Right-click Menu ///
    ////////////////////////
    //Main Menu
    Menu {
        id: contextMenu
        modal: true
        width: rightClickMenuWidth

        //Center view
        MenuItem{
            id: centerViewItem

            contentItem: Label {
                text: "Center View..."
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                centerViewSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + centerViewItem.y)
            }
        }

        //Follow boat
        MenuItem {
            enabled: boatPositionReceived

            contentItem: Label {
                text: followBoatText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: followBoat = !followBoat
        }

        //Zoom
        MenuItem {
            id: zoomItem

            contentItem: Label {
                text: "Zoom..."
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                zoomSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + zoomItem.y)
            }
        }

        //View up
        MenuItem {
            contentItem: Label {
                text: {
                    switch (mapViewMode)
                    {
                        case 0: default: return "Heading Up"
                        case 1: return "Course Up"
                        case 2: return "North Up"
                    }
                }
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                mapViewMode = (mapViewMode + 1) % 3
            }
        }


        //Markers
        MenuItem{
            id: markersItem

            contentItem: Label {
                text: "Markers..."
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                markerSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + markersItem.y)
            }
        }

        //UI Visibility
        MenuItem {
            contentItem: Label {
                text: showUI ? "Hide UI" : "Show UI"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: showUI = !showUI
        }
    }



    ////////////////////////////
    /// Right-click Submenus ///
    ////////////////////////////
    //Center View
    Menu {
        id: centerViewSubmenu
        width: rightClickMenuWidth/1.2
        modal: true

        MenuItem {
            enabled: boatPositionInit

            contentItem: Label {
                text: "On Boat"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                setCenterPositionOnBoat()
                goToZoomLevelMap(15)
            }
        }

        MenuItem {
            contentItem: Label {
                text: "On Cursor"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: setCenterPosition(cursorLatitude, cursorLongitude)
        }

        MenuItem {
            contentItem: Label {
                text: "On Position"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: centerViewDialog.open()
        }
    }

    //Markers
    Menu {
        id: markerSubmenu
        width: rightClickMenuWidth/0.8
        modal: true

        MenuItem {
            contentItem: Label {
                text: "Drop Marker On Cursor"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: addMarkerOnMap(cursorLatitude, cursorLongitude)
        }

        MenuItem{
            enabled: boatPositionInit

            contentItem: Label {
                text: "Drop Marker On Boat"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: {
                addMarkerOnMap(boatLatitude, boatLongitude)
            }
        }

        MenuItem {
            contentItem: Label {
                text: "Drop Marker On Position"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: dropMarkerDialog.open()
        }

        MenuItem {
            id: clearMarkersItem
            enabled: userMarkerCount > 0
            contentItem: Label {
                text: "Clear Markers"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
                color: clearMarkersItem.enabled ? "#ffffff" : "#808080"
            }
            onTriggered: clearMarkers()
        }
    }

    //Zoom
    Menu {
        id: zoomSubmenu
        width: rightClickMenuWidth
        modal: true

        MenuItem {
            contentItem: Label {
                text: "Maximum Zoom"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(map.maximumZoomLevel)
            }
        }

        MenuItem {
            contentItem: Label {
                text: "Minimum Zoom"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(map.minimumZoomLevel)
            }
        }

        MenuItem {
            contentItem: Label {
                text: "Close View"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(17)
            }
        }

        MenuItem {
            contentItem: Label {
                text: "Wide View"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(12)
            }
        }

        MenuItem {
            contentItem: Label {
                text: "Zoom+"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(mapZoomLevel+1)
            }
        }

        MenuItem {
            contentItem: Label {
                text: "Zoom-"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                goToZoomLevelMap(mapZoomLevel-1)
            }
        }
    }



    ////////////////////
    /// Dialog boxes ///
    ////////////////////
    Dialog {
        id: centerViewDialog
        modal: false
        closePolicy: Popup.NoAutoClose
        title: "Center View On Position"
        standardButtons: Dialog.Ok | Dialog.Cancel

        x: (parent.width  - width)  / 2
        y: (parent.height - height) / 2

        background: Rectangle {
            color: "white"
            radius: 12
        }

        Column {
            spacing: 10
            padding: 20

            Label { text: "Latitude:" }
            TextField {
                id: latInput
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label { text: "Longitude:" }
            TextField {
                id: lonInput
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label {
                id: errorLabel
                text: ""
                color: "red"
                visible: text !== ""
            }
        }

        onAccepted: {
            var lat = parseFloat(latInput.text)
            var lon = parseFloat(lonInput.text)

            if (isPositionValid(lat,lon)){
                setCenterPosition(lat, lon)
                errorLabel.text = ""
            }
            else{
                errorLabel.text = "Wrong input"
                centerViewDialog.acc
            }
        }
    }

    Dialog {
        id: dropMarkerDialog
        modal: false
        closePolicy: Popup.NoAutoClose
        title: "Drop Marker On Position"
        standardButtons: Dialog.Ok | Dialog.Cancel

        x: (parent.width  - width)  / 2
        y: (parent.height - height) / 2

        background: Rectangle {
            color: "white"
            radius: 12
        }

        Column {
            spacing: 10
            padding: 20

            Label { text: "Latitude:" }
            TextField {
                id: latInputMarker
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label { text: "Longitude:" }
            TextField {
                id: lonInputMarker
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label {
                id: errorLabelMarker
                text: ""
                color: "red"
                visible: text !== ""
            }
        }

        onAccepted: {
            var lat = parseFloat(latInputMarker.text)
            var lon = parseFloat(lonInputMarker.text)

            if (isPositionValid(lat,lon)){
                addMarkerOnMap(lat, lon)
                errorLabel.text = ""
            }
            else{
                errorLabel.text = "Wrong input"
                centerViewDialog.acc
            }
        }
    }



    ////////////////////////
    /// Marker Component ///
    ////////////////////////
    //Markers
    Component {
        id: redMarkerImg

        MapQuickItem {
            anchorPoint.x: image.width / 2
            anchorPoint.y: image.height
            coordinate: position

            sourceItem: Image {
                id: image
                width: 30
                height: 30
                source: "qrc:/markers/redMarker"
            }
        }
    }

    //Main boat icon
    Component {
        id: boatMarkerImg

        MapQuickItem {
            anchorPoint.x: image.width / 2
            anchorPoint.y: image.height / 2
            coordinate: position

            sourceItem: Image {
                id: image
                width: 20
                height: 60
                source: "qrc:/boats/boat1"
                opacity: elapsedSec > timeBeforePositionLost ? 0.5 : 1.0
            }
        }
    }



    //////////////
    /// Timers ///
    //////////////
    //Boat Date
    Timer {
        id: updateLastDateTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastUtcDate === 0)
                return

            elapsedSec = (Date.now() - timeLastUtcDate) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatDateReceived = false
        }
    }

    //Boat Time
    Timer {
        id: updateLastTimeTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastUtcTime === 0)
                return

            elapsedSec = (Date.now() - timeLastUtcTime) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatTimeReceived = false
        }
    }

    //Boat Position
    Timer {
        id: updateLastPositionTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastPosition === 0) {
                return
            }

            elapsedSec = (Date.now() - timeLastPosition) / 1000

            if(elapsedSec <= timeBeforePositionLost)
                if(elapsedSec < 1)
                    textTimerPositionUpdate = "Position Updated\n < 1s ago"
                else
                    textTimerPositionUpdate = "Position Updated\n"+ Math.ceil(elapsedSec) + "s ago"
            else{
                boatPositionReceived = false
                textTimerPositionUpdate = "Position Lost"
                followBoat = false
            }
        }
    }

    //Boat Heading
    Timer {
        id: updateLastHeadingTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastHeading === 0)
                return

            elapsedSec = (Date.now() - timeLastHeading) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatHeadingReceived = false
        }
    }

    //Boat Course
    Timer {
        id: updateLastCourseTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastCourse === 0)
                return

            elapsedSec = (Date.now() - timeLastCourse) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatCourseReceived = false
        }
    }

    //Boat Speed
    Timer {
        id: updateLastSpeedTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastSpeed === 0)
                return

            elapsedSec = (Date.now() - timeLastSpeed) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatSpeedReceived = false
        }
    }

    //Boat Depth
    Timer {
        id: updateLastDepthTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastDepth === 0)
                return

            elapsedSec = (Date.now() - timeLastDepth) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatDepthReceived = false
        }
    }

    //Boat WaterTemp
    Timer {
        id: updateLastWaterTempTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastWaterTemp === 0)
                return

            elapsedSec = (Date.now() - timeLastWaterTemp) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                boatWaterTemperatureReceived = false
        }
    }

    //Update boat icon on map
    Timer {
        id: updateMapViewOnBoatTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (followBoat){
                setCenterPositionOnBoat()
            }
        }
    }



    ///////////////////////////////
    /// Data Labels / Left Side ///
    ///////////////////////////////
    Column {
        id: leftSideInfoColumn
        visible: showUI

        anchors.top: parent.top
        anchors.topMargin: labelVerticalMargin * 2
        anchors.left: parent.left
        anchors.leftMargin: labelLateralMargin
        spacing: labelVerticalMargin

        // Map type
        Label {
            id: mapLabel
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: "Chart: OSM "
        }

        // Zoom level
        Label {
            id: zoomLabel
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: "Zoom Level: " + mapZoomLevel.toFixed(1)
        }

        // Map View Mode
        Label {
            id: mapViewModeLabel
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius: labelBackgroundRadius
            }
            font.pixelSize: 14

            text: {
                switch (mapViewMode) {
                case 1: return "Heading Up"
                case 2: return "Course Up"
                default: return "North Up"
                }
            }
        }


        // Cursor position
        Label {
            id: cursorPosition
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: "Cursor Position" + "\n" +
                   "Lat: " + formatLat(cursorLatitude) + "\n" +
                   "Lon: " + formatLon(cursorLongitude)
        }

        // Cursor distance, bearing and ETA from boat
        Label {
            id: distanceBearinFromBoat
            color: labelColor
            width: labelLeftSideWidth
            visible: boatPositionInit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: "From Boat" + "\n" +
                  "Distance: " + (metersToNauticalMiles(cursorDistanceBoat) > 100
                                 ? metersToNauticalMiles(cursorDistanceBoat).toFixed(0)
                                 : metersToNauticalMiles(cursorDistanceBoat).toFixed(2)) + "NM" + "\n" +
                  "Bearing: " + cursorBearingBoat.toFixed(0) + "°" + "\n" +
                  "ETA: " + secondsToDHMS(getETA(cursorDistanceBoat, boatSpeed))
        }

        // Last Time Position Update
        Label {
            id: elapsedLabel
            color: labelColor
            width: labelLeftSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: elapsedSec > timeBeforePositionLost ? "indianred" : labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: 14
            text: textTimerPositionUpdate
        }
    }



    ////////////////////////////////
    /// Data Labels / Right Side ///
    ////////////////////////////////
    Column {
        id: rightSideInfoColumn
        visible: showUI

        anchors.top: parent.top
        anchors.topMargin: labelVerticalMargin * 2
        anchors.right: parent.right
        anchors.rightMargin: labelLateralMargin
        spacing: labelVerticalMargin

        // Boat Date
        Label {
            id: dateLabel
            color: labelColor
            visible: boatDateReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatDateReceived ? "Date: " + boatDate
                                   : "Date: "+ noData
        }

        // Boat Time
        Label {
            id: timeLabel
            color: labelColor
            visible: boatTimeReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatTimeReceived ? "Time: " + boatTime
                                   : "Time: "+ noData
        }

        // Boat Position
        Label {
            id: positionLabel
            color: labelColor
            visible: boatPositionInit
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatPositionReceived ? "Boat Position\nLat: " + formatLat(boatLatitude) + "\nLon: " + formatLon(boatLongitude)
                                       : "Last Boat Position\nLat: " + formatLat(boatLatitude) + "\nLon: " + formatLon(boatLongitude)
        }

        // Heading
        Label {
            id: headingLabel
            color: labelColor
            visible: boatHeadingReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatHeadingReceived ? "Heading: " + boatHeading.toFixed(1) + "°"
                                      : "Heading: " + noData
        }

        // Course
        Label {
            id: courseLabel
            color: labelColor
            visible: boatCourseReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatCourseReceived ? "Course: " + boatCourse.toFixed(1) + "°"
                                     : "Course: " + noData
        }

        // Speed
        Label {
            id: speedLabel
            color: labelColor
            visible: boatSpeedReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatSpeedReceived ? "Speed: " + boatSpeed.toFixed(1) + "kts"
                                    : "Speed: " + noData
        }

        // Depth
        Label {
            id: depthLabel
            color: labelColor
            visible: boatDepthReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatDepthReceived ? "Depth: " + boatDepth.toFixed(1) + "m"
                                    : "Depth: " + noData
        }

        // Water Temperature
        Label {
            id: waterTemperatureLabel
            color: labelColor
            visible: boatWaterTemperatureReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: boatWaterTemperatureReceived ? "Water Temp: " + boatWaterTemperature.toFixed(1) + "°C"
                                              : "Water Temp: " + noData
        }
    }



    //////////////////////////////////
    /// View Actions / Bottom Left ///
    //////////////////////////////////
    Row{
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: labelLateralMargin
        anchors.bottomMargin: labelVerticalMargin
        spacing: labelVerticalMargin
        visible: showUI

        Column{
            spacing: labelVerticalMargin/2

            Row {
                spacing: labelLateralMargin

                //Zoom-
                Button {
                    text: "Zoom -"
                    width: 60
                    height: 30
                    onClicked: goToZoomLevelMap(mapZoomLevel-1)
                }

                //Zoom+
                Button {
                    text: "Zoom +"
                    width: 60
                    height: 30
                    onClicked: goToZoomLevelMap(mapZoomLevel+1)
                }
            }

            //Zoom slider
            Slider {
                id: zoomSlider
                from: map.minimumZoomLevel
                to: map.maximumZoomLevel
                stepSize: 0.1
                value: mapZoomLevel
                width: 130

                onValueChanged: goToZoomLevelMap(value)
            }
        }

        //Follow Boat
        Button {
            width: 60
            height: 60
            anchors.verticalCenter: parent.verticalCenter
            enabled: boatPositionReceived
            text: followBoatText.replace(" ", "\n")

            onClicked: followBoat = !followBoat
        }
    }




    ///////////////////
    /// Data Canvas ///
    ///////////////////
    Canvas {
        id: compassCanvas

        width: 150
        height: 150
        visible: (boatHeadingReceived && showUI)

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: labelVerticalMargin
        anchors.rightMargin: labelLateralMargin

        property real heading: boatHeading
        property real course: boatCourse

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var centerX = width / 2
            var centerY = height / 2
            var radius = Math.min(width, height) / 2 - 10

            // Draw compass circle
            ctx.beginPath()
            ctx.strokeStyle = "black"
            ctx.lineWidth = 2
            ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI)
            ctx.stroke()

            // Draw cardinal directions
            ctx.fillStyle = "black"
            ctx.font = "bold 14px sans-serif"
            ctx.textAlign = "center"
            ctx.textBaseline = "middle"
            ctx.fillText("N",  centerX, centerY - radius + 10);
            ctx.fillText("E",  centerX + radius - 10, centerY);
            ctx.fillText("S",  centerX, centerY + radius - 10);
            ctx.fillText("W",  centerX - radius + 10, centerY);

            // Intercardinal directions
            ctx.font = "10px sans-serif"
            ctx.fillText("NE", centerX + radius * 0.55, centerY - radius * 0.55);
            ctx.fillText("SE", centerX + radius * 0.55, centerY + radius * 0.55);
            ctx.fillText("SW", centerX - radius * 0.55, centerY + radius * 0.55);
            ctx.fillText("NW", centerX - radius * 0.55, centerY - radius * 0.55);

            // Draw heading arrow
            ctx.save()
            ctx.translate(centerX, centerY)
            ctx.rotate((heading - 0) * Math.PI / 180)

            ctx.beginPath()
            ctx.moveTo(0, -radius + 15)
            ctx.lineTo(5, 0)
            ctx.lineTo(-5, 0)
            ctx.closePath()

            ctx.fillStyle = "red"
            ctx.fill()
            ctx.restore()

            // Draw course arrow
            ctx.save()
            ctx.translate(centerX, centerY)
            ctx.rotate((course - 0) * Math.PI / 180)

            ctx.beginPath()
            ctx.moveTo(0, -radius + 20)
            ctx.lineTo(3, 0)
            ctx.lineTo(-3, 0)
            ctx.closePath()

            ctx.fillStyle = "blue"
            ctx.fill()
            ctx.restore()

            // Draw center black circle
            ctx.beginPath()
            ctx.arc(centerX, centerY, 5, 0, 2 * Math.PI)
            ctx.fillStyle = "black"
            ctx.fill()
        }

        // Redraw when heading changes
        Connections {
            target: compassCanvas

            function onHeadingChanged() {
                compassCanvas.requestPaint()
            }

            function onCourseChanged() {
                compassCanvas.requestPaint()
            }
        }
    }



    //////////////////
    /// Update Map ///
    //////////////////
    //Go To New Position
    function setCenterPosition(targetLat, targetLon) {
        map.center = QtPositioning.coordinate(targetLat, targetLon)
    }

    //Go To New Position
    function setCenterPositionOnBoat() {
        map.center = QtPositioning.coordinate(boatLatitude, boatLongitude)
    }

    //Add marker
    function addMarkerOnMap(lat, lon) {
        var item = redMarker.createObject(window, {
            coordinate: QtPositioning.coordinate(lat, lon),
            objectName: "marker"
        });
        map.addMapItem(item)

        userMarkerCount++;
    }

    //Add boat icon
    function updateBoatIconOnMap() {
        // Remove previous boat icon if it exists
        if (boatMarkerRef !== null) {
            map.removeMapItem(boatMarkerRef)
            boatMarkerRef.destroy()
            boatMarkerRef = null

        }

        // Create and store new boat marker
        boatMarkerRef = boatMarker.createObject(window, {
            coordinate: QtPositioning.coordinate(boatLatitude, boatLongitude),
            rotation: boatHeading - mapRotation
        })

        map.addMapItem(boatMarkerRef)
    }

    //Remove all markers from map
    function clearMarkers() {
        for (var i = map.mapItems.length - 1; i >= 0; i--) {
            var item = map.mapItems[i];
            if (item.objectName === "marker") { //Remove all markers with objectName: "marker"
                map.removeMapItem(item);
            }
        }

        userMarkerCount = 0;
    }

    //Increment Map Zoom
    function incrementZoomMap(dz) {
        if (dz > 0)
            map.zoomLevel = Math.min(map.maximumZoomLevel, map.zoomLevel + dz);
        else if (dz < 0)
            map.zoomLevel = Math.max(map.minimumZoomLevel, map.zoomLevel + dz);
        mapZoomLevel = map.zoomLevel
    }

    //Go To Zoom Level Map
    function goToZoomLevelMap(zoomLevel) {
        if(zoomLevel > map.maximumZoomLevel)
            zoomLevel = map.maximumZoomLevel
        else if(zoomLevel < map.minimumZoomLevel)
            zoomLevel = map.minimumZoomLevel

        map.zoomLevel = zoomLevel;
        mapZoomLevel = map.zoomLevel
    }



    ///////////////////
    /// Update Data ///
    ///////////////////
    //Update boat UTC time
    function updateBoatTime(time) {
        boatTime = time

        timeLastUtcTime = Date.now()
        boatTimeReceived = true
    }

    //Update boat UTC Date
    function updateBoatDate(date) {
        boatDate = date
        timeLastUtcDate = Date.now()
        boatDateReceived = true
    }

    //Update boat position
    function updateBoatPosition(lat, lon) {
        boatLatitude = lat
        boatLongitude = lon

        timeLastPosition = Date.now()
        boatPositionReceived = true
        boatPositionInit = true

        updateBoatIconOnMap()
    }

    //Update boat heading
    function updateBoatHeading(head) {
        boatHeading = head

        timeLastHeading= Date.now()
        boatHeadingReceived = true
    }

    //Update boat depth
    function updateBoatDepth(depth) {
        boatDepth = depth

        timeLastDepth = Date.now()
        boatDepthReceived = true
    }

    //Update boat speed
    function updateBoatSpeed(speed) {
        boatSpeed = speed

        timeLastSpeed = Date.now()
        boatSpeedReceived = true
    }

    //Update boat course
    function updateBoatCourse(course) {
        boatCourse = course

        timeLastCourse= Date.now()
        boatCourseReceived = true
    }

    //Update boat water temperature
    function updateBoatWaterTemperature(temp) {
        boatWaterTemperature = temp

        timeLastWaterTemp = Date.now()
        boatWaterTemperatureReceived = true
    }

    //Recalculate cursor coordinate relative to mouse position
    function updateCursorCalculations() {
        var coord = map.toCoordinate(Qt.point(mouseArea.mouseX, mouseArea.mouseY))
        cursorLatitude = coord.latitude
        cursorLongitude = coord.longitude

        cursorDistanceBoat = haversineDistance(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
        cursorBearingBoat = calculateBearing(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
    }



    ///////////////////
    /// Conversions ///
    ///////////////////
    function toRadians(deg) {
        return deg * Math.PI / 180.0
    }

    function toDegrees(rad) {
        return rad * 180.0 / Math.PI
    }

    function metersToNauticalMiles(meters) {
        return meters / 1852.0
    }

    function knotsToMps(speedKnots) {
        return speedKnots * 0.514444;
    }



    //////////////
    /// Format ///
    //////////////
    function formatLat(lat) {
        return Math.abs(lat).toFixed(5) + "°" + (lat >= 0 ? "N" : "S")
    }

    function formatLon(lon) {
        return Math.abs(lon).toFixed(5) + "°" + (lon >= 0 ? "E" : "W")
    }



    /////////////////////////
    /// Generic Functions ///
    /////////////////////////
    //Check if position is valid
    function isPositionValid(lat, lon) {
        if (isNaN(lat) || isNaN(lon))
            return false

        return lat >= -90 && lat <= 90 && lon >= -180 && lon <= 180
    }

    //Distance between 2 positions
    function haversineDistance(lat1, lon1, lat2, lon2) {
        const R = 6378137.0; // Earth radius in meters

        let dLat = toRadians(lat2 - lat1)
        let dLon = toRadians(lon2 - lon1)

        lat1 = toRadians(lat1)
        lat2 = toRadians(lat2)

        let a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
                Math.cos(lat1) * Math.cos(lat2) *
                Math.sin(dLon / 2) * Math.sin(dLon / 2)

        let c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a))

        return R * c // in meters
    }

    //Bearing between 2 positions
    function calculateBearing(lat1, lon1, lat2, lon2) {
        lat1 = toRadians(lat1)
        lon1 = toRadians(lon1)
        lat2 = toRadians(lat2)
        lon2 = toRadians(lon2)

        let dLon = lon2 - lon1
        let y = Math.sin(dLon) * Math.cos(lat2)
        let x = Math.cos(lat1) * Math.sin(lat2) -
                Math.sin(lat1) * Math.cos(lat2) * Math.cos(dLon)

        let bearing = toDegrees(Math.atan2(y, x))
        return (bearing + 360) % 360  // Normalize to 0–359°
    }

    //Get Estimated Time of Arrival
    function getETA(distanceFromBoat, speed) {
        if (speed === 0)
            return NaN;

        return distanceFromBoat/knotsToMps(boatSpeed) // meter / mps = seconds
    }

    //Convert seconds to hours/minutes/seconds
    function secondsToDHMS(seconds) {
        if (isNaN(seconds))
            return "N/A";

        var d = Math.floor(seconds / 86400);
        var h = Math.floor((seconds % 86400) / 3600);
        var m = Math.floor((seconds % 3600) / 60);
        var s = Math.floor(seconds % 60);

        // Show days, hours, minutes only
        if (d > 0) {
            return (d > 0 ? d + "d " : "") +
                   (h > 0 ? h + "h " : "") +
                   (m > 0 ? m + "m" : "");
        }
        // Show h, m, s normally
        else {
            return (h > 0 ? h + "h " : "") +
                   (m > 0 ? m + "m " : "") +
                   s + "s";
        }
    }

    //Calculate end position when adding distance to initial position/heading
    function destinationCoordinate(lat, lon, bearingDeg, distanceMeters) {
        const R = 6378137.0; // Earth radius in meters
        const brng = toRadians(bearingDeg)
        const phi_1 = lat * Math.PI / 180
        const lambda_1 = lon * Math.PI / 180

        const phi_2 = Math.asin(Math.sin(phi_1) * Math.cos(distanceMeters / R) +
                             Math.cos(phi_1) * Math.sin(distanceMeters / R) * Math.cos(brng))
        const lambda_2 = lambda_1 + Math.atan2(Math.sin(brng) * Math.sin(distanceMeters / R) * Math.cos(phi_1),
                                   Math.cos(distanceMeters / R) - Math.sin(phi_1) * Math.sin(phi_2))

        return QtPositioning.coordinate(phi_2 * 180 / Math.PI, lambda_2 * 180 / Math.PI)
    }
}

