
////////////////////////
/// Import libraries ///
////////////////////////

//Qt
import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning


//Javascript
import "utils.js" as Utils


//Main code
Item {
    id: window

    ////////////////////////
    /// Global variables ///
    ////////////////////////

    //Position
    property double mapCenterInitLatitude:  35
    property double mapCenterInitLongitude: 0
    property var currentBoatCoord: null
    property var panZoomAnimCoord: null
    property bool boatPositionInit: false

    //Cursor
    property var cursorCoord: NaN
    property double cursorLatitude: NaN
    property double cursorLongitude: NaN
    property double cursorDistanceBoat: NaN
    property double cursorBearingBoat: NaN

    //Received data
    property string noData: qsTr("No Data")
    property string date: ""
    property string time: ""
    property double boatLatitude: 0
    property double boatLongitude: 0
    property double boatHeading: 0      //°
    property double boatCourse: 0       //°
    property double depth: 0            //meters
    property double boatSpeed: 0        //knots
    property double waterTemperature: 0 //°C
    property double satellitesInView: 0
    property double windAngle: 0        //°
    property double windSpeed: 0        //kts

    //Received data check
    property bool dateReceived: false
    property bool timeReceived: false
    property bool boatPositionReceived: false
    property bool boatHeadingReceived: false
    property bool boatCourseReceived: false
    property bool depthReceived: false
    property bool boatSpeedReceived: false
    property bool waterTemperatureReceived: false
    property bool satellitesReceived: false
    property bool windReceived: false

    //Labels
    property int rightClickMenuWidth: 150
    property int labelRightSideWidth: 135
    property int labelLeftSideWidth: 150
    property int labelPadding: 8
    property int labelLateralMargin: 8
    property int labelVerticalMargin: 8
    property int labelFontSize: 14
    property int labelBackgroundRadius: 4
    property string labelColor: "white"
    property string labelBackgroundColor: "grey"

    //Zoom
    property double zoomIncrement: 0.2

    //Markers
    property int userMarkerCount: 0
    property var boatMarkerRef: null

    //Map
    property bool showWidgets: true
    property bool followBoat: false

    // View modes
    property double wheelDragRotateSensitivity: 0.015
    property double wheelDragTiltSensitivity: 0.15

    property int mapViewMode: 0
    property double freeViewUp: 0
    property real mapRotation: {
        switch (mapViewMode)
        {
            case 0: default: return 0   // North Up
            case 1: return boatHeading  // Heading Up
            case 2: return boatCourse   // COG Up
            case 3: return freeViewUp   // Free view
        }
    }

    //Timer Data Update
    property int timeBeforePositionLost: 10   //seconds
    property int timeBeforeGeneralDataLost: 10 //seconds
    property string textTimerPositionUpdate: qsTr("No Position Data")

    property double timeLastUtcDate: 0
    property double timeLastUtcTime: 0
    property double timeLastPosition: 0
    property double timeLastHeading: 0
    property double timeLastCourse: 0
    property double timeLastSpeed: 0
    property double timeLastDepth: 0
    property double timeLastWaterTemp: 0
    property double timeLastSatellites: 0
    property double timeLastWind: 0
    property double elapsedSec: 0

    //Heading & COG lines
    property int distanceLineTimeBoatTrip: 300 //seconds
    property int distanceLineTimeWindTrip: 120 //seconds
    property double cogLineDistance: Utils.knotsToMps(boatSpeed) * distanceLineTimeBoatTrip
    property double headingLineDistance: Utils.knotsToMps(boatSpeed) * distanceLineTimeBoatTrip * Math.cos(Utils.toRadians(boatCourse - boatHeading))
    property double windLineDistance: Utils.knotsToMps(windSpeed) * distanceLineTimeWindTrip

    //Boat Track
    property bool enableTrack: false
    property int minimumTrackPointsDistance: 50 //meters
    property var boatTrack: []
    property int maxTrackPoints: 500
    property int trackLineWidth: 10
    property double trackLineOpacity: 0.6
    property color trackLineColor: Qt.rgba(1, 1, 0, trackLineOpacity)

    //Measure mode
    property bool measureMode: false
    property var measurePoint: null
    property var measureTrack: []
    property real measureTotalMeters: 0

    //Map overlay
    property double mapOverlayOpacity: 1
    property bool openSeaMapEnabled: true


    //-------------------------------------------------------------------------------//


    ///////////////////
    /// Map Sources ///
    ///////////////////

    //OpenTopoMap
    Plugin {
        id: osmPlugin
        name: "osm"

        PluginParameter {name: "osm.mapping.host"; value: "https://a.tile.opentopomap.org/"}
        PluginParameter {name: "osm.mapping.cache.directory"; value: "cache" }
        PluginParameter {name: "osm.mapping.cache.disk.size"; value: 0 }
        PluginParameter {name: "osm.mapping.providersrepository.disabled"; value: true} //Disable Qt's default provider
    }

    //OpenSeaMap overlay
    Plugin {
        id: openSeaMapPlugin
        name: "osm"

        PluginParameter {name: "osm.mapping.custom.host"; value: "https://tiles.openseamap.org/seamark/"}
        PluginParameter {name: "osm.mapping.cache.directory"; value: "cache" }
        PluginParameter {name: "osm.mapping.cache.disk.size"; value: 0 }
        PluginParameter {name: "osm.mapping.providersrepository.disabled"; value: true} //Disable Qt's default provider
    }



    ////////////
    /// Maps ///
    ////////////

    //Main map
    Map {
        id: map
        anchors.fill: parent
        center: QtPositioning.coordinate(mapCenterInitLatitude, mapCenterInitLongitude)
        zoomLevel: map.minimumZoomLevel
        activeMapType: map.supportedMapTypes[map.supportedMapTypes.length - 1]
        bearing: mapRotation

        //Load OSM plugin
        plugin: osmPlugin

        //Track Line
        MapPolyline { //Lines between existing waypoints
            visible: boatTrack.length > 1
            line.width: trackLineWidth
            line.color: trackLineColor
            path: boatTrack
        }
        MapPolyline { //Line from last waypoint to boat
            line.width: trackLineWidth
            line.color: trackLineColor

            path: {
                if (!enableTrack)
                    return []
                if (boatTrack.length === 0)
                    return []
                if (!currentBoatCoord)
                    return []

                return [
                    boatTrack[boatTrack.length - 1],
                    currentBoatCoord
                ]
            }
        }

        //COG Line
        MapPolyline {
            visible: (boatPositionReceived && boatCourseReceived)
            line.width: 3
            line.color: "blue"

            path: [
                QtPositioning.coordinate(boatLatitude, boatLongitude),
                Utils.destinationCoordinate(boatLatitude, boatLongitude, boatCourse, cogLineDistance)
            ]
        }
        MapQuickItem {
            visible: (boatPositionReceived && boatCourseReceived)
            coordinate: Utils.destinationCoordinate(boatLatitude, boatLongitude, boatCourse, cogLineDistance)
            anchorPoint.x: 5
            anchorPoint.y: 5
            sourceItem: Rectangle {
                width: 10
                height: 10
                radius: 5
                color: "blue"
                border.color: "black"
                border.width: 1
            }
        }

        //Heading Line
        MapPolyline {
            visible: (boatPositionReceived && boatHeadingReceived)
            line.width: 3
            line.color: "red"

            path: [
                QtPositioning.coordinate(boatLatitude, boatLongitude),
                Utils.destinationCoordinate(boatLatitude, boatLongitude, boatHeading, headingLineDistance)
            ]
        }
        MapQuickItem {
            visible: (boatPositionReceived && boatHeadingReceived)
            coordinate: Utils.destinationCoordinate(boatLatitude, boatLongitude, boatHeading, headingLineDistance)
            anchorPoint.x: 3
            anchorPoint.y: 3
            sourceItem: Rectangle {
                width: 10
                height: 10
                radius: 5
                color: "red"
                border.color: "black"
                border.width: 1
            }
        }

        //Wind Line
        MapPolyline {
            visible: boatPositionReceived && windReceived
            line.width: 3
            line.color: "green"

            path: [
                QtPositioning.coordinate(boatLatitude, boatLongitude),
                Utils.destinationCoordinate(boatLatitude, boatLongitude, (boatHeading + windAngle + 360) % 360, windLineDistance)
            ]
        }
        MapQuickItem {
            visible: boatPositionReceived && windReceived
            coordinate: Utils.destinationCoordinate(boatLatitude, boatLongitude, (boatHeading + windAngle + 360) % 360, windLineDistance)

            anchorPoint.x: 4
            anchorPoint.y: 4

            sourceItem: Rectangle {
                width: 8
                height: 8
                radius: 4
                color: "green"
                border.color: "black"
                border.width: 1
            }
        }

        //Measure Distance Line
        MapPolyline {
            visible: measureMode && measureTrack.length > 0
            line.width: 2
            line.color: "black"

            path: {
                if (measureTrack.length === 0) return []
                var last = measureTrack[measureTrack.length - 1]
                var current = cursorCoord
                return [last, current]
            }
        }
        MapPolyline {
            visible: measureMode && measureTrack.length > 1
            line.width: 2
            line.color: "black"

            path: measureTrack.length > 0 ? measureTrack : []
        }
    }

    // Overlay
    Map {
        id: openSeaMapLayer
        anchors.fill: parent
        plugin: openSeaMapPlugin

        // Overlay behavior
        visible: openSeaMapEnabled
        enabled: openSeaMapEnabled
        opacity: mapOverlayOpacity
        color: "transparent"

        // Sync camera with base map
        center: map.center
        zoomLevel: map.zoomLevel
        bearing: map.bearing
        tilt: map.tilt

        // Force custom map type
        activeMapType: supportedMapTypes.find(t => t.style === MapType.CustomMap)
    }



    //////////////////////////
    /// Keyboard Shortcuts ///
    //////////////////////////
    Shortcut { //Follow boat
        sequence: "F"
        onActivated: followBoat = !followBoat
        enabled: boatPositionReceived
    }

    Shortcut { //Pan + Zoom On Boat
        sequence: "Z"
        onActivated: goToPositionAnimation(currentBoatCoord, 15)
        enabled: boatPositionInit
    }

    Shortcut { //Close View
        sequence: "Ctrl++"
        onActivated: mapGoToZoomLevelSmooth(17)
    }

    Shortcut { //Wide View
        sequence: "Ctrl+-"
        onActivated: mapGoToZoomLevelSmooth(12)
    }

    Shortcut { //Zoom In
        sequence: "+"
        onActivated: mapGoToZoomLevel(map.zoomLevel + 1)
    }

    Shortcut { //Zoom out
        sequence: "-"
        onActivated: mapGoToZoomLevel(map.zoomLevel - 1)
    }

    Shortcut { //Hide UI
        sequence: "H"
        onActivated: showWidgets = !showWidgets
    }

    Shortcut { //Enter Measure Mode
        sequence: "D"
        onActivated: measureMode = !measureMode
    }

    Shortcut { //Quit Measure Mode
        sequence: "Escape"
        onActivated: {
            if(measureMode){
                measureMode = false
                measurePoint = null
            }
        }
    }

    Shortcut { //Switch View Up
        sequence: "V"
        onActivated: {
            mapViewMode = (mapViewMode + 1) % 4 //cycle through all 4 modes

            if(mapViewMode == 0)
                freeViewUp = 0
            else if(mapViewMode == 1)
                freeViewUp = boatHeading
            else if(mapViewMode == 2)
                freeViewUp = boatCourse
        }
    }

    Shortcut { //Drop Marker On Boat
        sequence: "M"
        onActivated: addMarkerOnMap(boatLatitude, boatLongitude)
    }

    Shortcut { //Enable Tracking
        sequence: "T"
        onActivated: enableTrack = !enableTrack
    }

    Shortcut { //Clear Track
        sequence: "C"
        onActivated: boatTrack = []
    }



    /////////////////////
    /// Mouse Control ///
    /////////////////////

    //// WheelHandler ////
    WheelHandler {
        target: map

        onWheel: function(event) { //Zoom on cursor
            //Coordinates before zoom
            var cursorPoint = Qt.point(event.x, event.y)
            var cursorCoordBefore = map.toCoordinate(cursorPoint)

            //Zoom
            if (event.angleDelta.y > 0)
                map.zoomLevel = Math.min(map.maximumZoomLevel, map.zoomLevel + zoomIncrement)
            else if (event.angleDelta.y < 0)
                map.zoomLevel = Math.max(map.minimumZoomLevel, map.zoomLevel - zoomIncrement)

            //Coordinates after zoom
            var cursorCoordAfter = map.toCoordinate(cursorPoint)
            var latShift = cursorCoordBefore.latitude - cursorCoordAfter.latitude
            var lonShift = cursorCoordBefore.longitude - cursorCoordAfter.longitude

            // Adjust map center so cursor stays fixed
            map.center = QtPositioning.coordinate(map.center.latitude + latShift, map.center.longitude + lonShift)
        }
    }


    //// Mouse Area ////
    MouseArea {
        id: mouseArea
        anchors.fill: map
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton

        property var lastCoord
        property bool dragging: false

        // Rotation / tilt state
        property bool rotating: false
        property real lastMouseX: 0
        property real lastMouseY: 0
        property real startTilt: 0

        onClicked: function(mouse) {
            // Right click
            if (mouse.button === Qt.RightButton) {
                if (measureMode) {
                    measureMode = false
                    measurePoint = null
                    measureTrack = []
                    measureTotalMeters = 0
                } else {
                    contextMenu.popup()
                }
            }
        }

        onPressed: function(mouse) {
            // Left button → pan / measure
            if (mouse.button === Qt.LeftButton) {
                if (measureMode) {
                    var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                    if (measureTrack.length > 0) {
                        var last = measureTrack[measureTrack.length - 1]
                        measureTotalMeters += Utils.haversineDistance(
                            last.latitude, last.longitude,
                            coord.latitude, coord.longitude
                        )
                    }
                    measureTrack = measureTrack.concat([coord])
                    measurePoint = measureTrack[0]
                } else {
                    lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                    dragging = true
                    followBoat = false
                }
            }

            // Middle button → rotate + tilt
            else if (mouse.button === Qt.MiddleButton) {
                rotating = true
                lastMouseX = mouse.x
                lastMouseY = mouse.y
                startTilt = map.tilt

            }
        }

        onReleased: function(mouse) {
            dragging = false
            rotating = false
        }

        onPositionChanged: function(mouse) {
            cursorCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))

            // Rotate (X) + Tilt (Y)
            if (rotating) {
                var dx = mouse.x - lastMouseX
                var dy = mouse.y - lastMouseY

                // Rotation: horizontal drag (only if freeView mode)
                if( mapViewMode == 3)
                    freeViewUp += dx* wheelDragRotateSensitivity

                // Tilt: vertical drag
                map.tilt = Math.max(0, Math.min(80, startTilt + dy * wheelDragTiltSensitivity))

                return
            }

            // Pan map
            if (!measureMode && dragging && mouse.buttons === Qt.LeftButton)
            {
                var currentCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                var dxCoord = lastCoord.longitude - currentCoord.longitude
                var dyCoord = lastCoord.latitude - currentCoord.latitude

                map.center = QtPositioning.coordinate(map.center.latitude + dyCoord, map.center.longitude + dxCoord)
                lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
            }
            else
            {
                lastCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
            }

            // Cursor info
            cursorLatitude = cursorCoord.latitude
            cursorLongitude = cursorCoord.longitude
            cursorDistanceBoat = Utils.haversineDistance(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
            cursorBearingBoat = Utils.calculateBearing(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
        }

        //Text next to cursor (in measure mode only)
        Text {
            x: mouseArea.mouseX + 10
            y: mouseArea.mouseY + 10
            font.pixelSize: 14
            color: "black"
            z: 999

            visible: measureMode && measureTrack.length !== 0

            text: {
                if (measureTrack.length === 0)
                    return ""

                // Distance and bearing from last waypoint to cursor
                var last = measureTrack[measureTrack.length - 1]
                var cursor = cursorCoord
                var distMeters = Utils.haversineDistance(
                    last.latitude, last.longitude,
                    cursor.latitude, cursor.longitude
                )
                var distStr = distMeters < 1000
                    ? Math.round(distMeters) + " m"
                    : (distMeters / 1000).toFixed(2) + " km"
                var distNM = Utils.metersToNauticalMiles(distMeters).toFixed(2) + " NM"
                var bearingStr = Utils.calculateBearing(
                    last.latitude, last.longitude,
                    cursor.latitude, cursor.longitude
                ).toFixed(0) + "°"

                // Compute total track distance (between waypoints + last waypoint to cursor)
                var trackMeters = distMeters
                for (var i = 1; i < measureTrack.length; ++i) {
                    trackMeters += Utils.haversineDistance(
                        measureTrack[i-1].latitude, measureTrack[i-1].longitude,
                        measureTrack[i].latitude, measureTrack[i].longitude
                    )
                }
                var trackStr = trackMeters < 1000
                    ? Math.round(trackMeters) + " m"
                    : (trackMeters / 1000).toFixed(2) + " km"
                var trackNM = Utils.metersToNauticalMiles(trackMeters).toFixed(2) + " NM"

                //Text
                return qsTr("Track Total: ") + trackStr + " / " + trackNM + "\n"
                     + qsTr("To Cursor: ") + distStr + " / " + distNM + "\n"
                     + qsTr("Bearing: ") + bearingStr
            }
        }
    }



    ////////////////////////
    /// Right-Click Menu ///
    ////////////////////////

    //// Menu ////
    Menu {
        id: contextMenu
        modal: true
        width: rightClickMenuWidth

        //View
        MenuItem{
            id: viewItem

            contentItem: Label {
                text: qsTr("View...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                viewSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + viewItem.y)
            }
        }

        //Zoom
        MenuItem {
            id: zoomItem

            contentItem: Label {
                text: qsTr("Zoom...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                zoomSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + zoomItem.y)
            }
        }

        //Map Overlay
        MenuItem {
            id: overlayItem

            contentItem: Label {
                text: qsTr("Map Overlay...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                overlaySubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + overlayItem.y)
            }
        }

        //Boat
        MenuItem {
            id: drawTrackItem
            contentItem: Label {
                text: qsTr("Boat...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                boatSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + drawTrackItem.y)
            }
        }

        //Markers
        MenuItem{
            id: markersItem

            contentItem: Label {
                text: qsTr("Markers...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                markerSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + markersItem.y)
            }
        }

        //Measure mode
        MenuItem {
            id: measureModeItem
            contentItem: Label {
                text: (measureMode ? qsTr("Stop Measuring") : qsTr("Measure Distance")) + " (D)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                measureMode = !measureMode
                if (!measureMode) {
                    measurePoint = null
                }
            }
        }

        MenuItem {
            id: settingsItem

            contentItem: Label {
                text: qsTr("Settings...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: settingsDialog.open()
        }
    }


    //// Submenus ////
    //View
    Menu {
        id: viewSubmenu
        width: rightClickMenuWidth/1.2
        modal: true

        //Reset view
        MenuItem{
            id: resetViewItem

            contentItem: Label {
                text: qsTr("Reset View")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                map.tilt = 0
                mapViewMode = 0
            }
        }

        //Center view
        MenuItem{
            id: centerViewItem

            contentItem: Label {
                text: qsTr("Center View...")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                centerViewSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + centerViewItem.y)
            }
        }

        //View up
        MenuItem {
            id: viewUpItem

            contentItem: Label {
                text: qsTr("View Up...") + " (V)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                viewUpSubmenu.popup(contextMenu.x + contextMenu.width, contextMenu.y + viewUpItem.y)
            }
        }

        //UI Visibility
        MenuItem {
            id: uiVisibilityItem

            contentItem: Label {
                text: (showWidgets ? qsTr("Hide Widgets") : qsTr("Show Widgets")) + " (H)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                showWidgets = !showWidgets
            }
        }
    }

    //Zoom
    Menu {
        id: zoomSubmenu
        width: rightClickMenuWidth
        modal: true

        MenuItem {
            contentItem: Label {
                text: qsTr("Maximum Zoom")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                mapGoToZoomLevelSmooth(map.maximumZoomLevel)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Minimum Zoom")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                mapGoToZoomLevelSmooth(map.minimumZoomLevel)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Close View") + " (Ctrl+)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                mapGoToZoomLevelSmooth(17)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Wide View") + " (Ctrl-)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                mapGoToZoomLevelSmooth(12)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Zoom In") + " (+)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                mapGoToZoomLevel(map.zoomLevel+1)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Zoom Out") + " (-)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered:{
                mapGoToZoomLevel(map.zoomLevel-1)
            }
        }
    }

    //Overlay
    Menu {
        id: overlaySubmenu
        width: rightClickMenuWidth
        modal: true

        MenuItem {
            contentItem: Label {
                text: openSeaMapEnabled ? "Disable" : "Enable"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: openSeaMapEnabled = !openSeaMapEnabled
        }

        MenuItem {
            enabled: openSeaMapEnabled

            contentItem: Column {
                width: parent.width
                spacing: 6

                Label {
                    text: qsTr("Opacity")
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                }

                Slider {
                    width: parent.width
                    from: 0.0
                    to: 1.0
                    stepSize: 0.05
                    value: mapOverlayOpacity

                    onValueChanged: mapOverlayOpacity = value
                }
            }
        }

    }

    //Markers
    Menu {
        id: markerSubmenu
        width: rightClickMenuWidth/0.8
        modal: true

        MenuItem {
            contentItem: Label {
                text: qsTr("Drop Marker On Cursor")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: addMarkerOnMap(cursorLatitude, cursorLongitude)
        }

        MenuItem{
            enabled: boatPositionInit

            contentItem: Label {
                text: qsTr("Drop Marker On Boat") + " (M)"
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
                text: qsTr("Drop Marker On Position")
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
                text: qsTr("Clear Markers")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: clearMarkers()
        }
    }

    //Boat
    Menu {
        id: boatSubmenu
        width: rightClickMenuWidth/1.2
        modal: true

        //Follow boat
        MenuItem {
            id: followBoatItem

            enabled: boatPositionReceived

            contentItem: Label {
                text: (followBoat ? qsTr("Unfollow Boat") : qsTr("Follow Boat")) + " (F)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                followBoat = !followBoat
            }
        }

        //Tracking
        MenuItem {
            contentItem: Label {
                text: (enableTrack ? qsTr("Disable Tracking") : qsTr("Enable Tracking")) + " (T)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: enableTrack = !enableTrack
        }

        //Clear track
        MenuItem {
            contentItem: Label {
                text: qsTr("Clear Track") + " (C)"
                enabled: boatTrack.length > 0
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: boatTrack = []
        }
    }


    //// Dialog Boxes ////
    Dialog {
        id: centerViewDialog
        modal: false
        closePolicy: Popup.NoAutoClose
        title: qsTr("Center View On Position")
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

            Label { text: qsTr("Latitude:") }
            TextField {
                id: latInput
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label { text: qsTr("Longitude:") }
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

            if (Utils.isPositionValid(lat,lon)){
                followBoat = false
                const positionCoord = QtPositioning.coordinate(lat, lon)
                goToPositionAnimation(positionCoord, 15)
                errorLabel.text = ""
            }
            else{
                errorLabel.text = qsTr("Wrong input")
                centerViewDialog.acc
            }
        }
    }

    Dialog {
        id: dropMarkerDialog
        modal: false
        closePolicy: Popup.NoAutoClose
        title: qsTr("Drop Marker On Position")
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

            Label { text: qsTr("Latitude:") }
            TextField {
                id: latInputMarker
                placeholderText: " "
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            Label { text: qsTr("Longitude:") }
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

            if (Utils.isPositionValid(lat,lon)){
                addMarkerOnMap(lat, lon)
                errorLabel.text = ""
            }
            else{
                errorLabel.text = qsTr("Wrong input")
                centerViewDialog.acc
            }
        }
    }

    Dialog {
        id: settingsDialog
        title: qsTr("Genral Settings")
        modal: true
        standardButtons: Dialog.Ok

        x: (parent.width  - width)  / 2
        y: (parent.height - height) / 2

        Column {
            spacing: 10
            padding: 10

            // Zoom Speed
            Label { text: qsTr("Zoom Speed") }

            Row {
                spacing: 10

                Slider {
                    id: zoomSpeedSlider
                    from: 0.01
                    to: 0.5
                    stepSize: 0.01
                    value: zoomIncrement
                    onValueChanged: zoomIncrement = value
                    width: 200
                }

                Label {
                    text: Math.round((zoomIncrement - 0.01) / (0.5 - 0.01) * 100) + " %"
                    width: 50
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // Rotate sensitivity
            Label { text: qsTr("Rotate Sensitivity") }
            Row {
                spacing: 10

                Slider {
                    id: rotateSlider
                    from: 0.001
                    to: 0.1
                    stepSize: 0.001
                    value: wheelDragRotateSensitivity
                    onValueChanged: wheelDragRotateSensitivity = value
                    width: 200
                }

                Label {
                    text: Math.round((wheelDragRotateSensitivity - 0.001) / (0.1 - 0.001) * 100) + " %"
                    width: 50
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // Tilt Sensitivity
            Label { text: qsTr("Tilt Sensitivity") }

            Row {
                spacing: 10

                Slider {
                    id: tiltSlider
                    from: 0.01
                    to: 0.5
                    stepSize: 0.01
                    value: wheelDragTiltSensitivity
                    onValueChanged: wheelDragTiltSensitivity = value
                    width: 200
                }

                Label {
                    text: Math.round((wheelDragTiltSensitivity - 0.01) / (0.5 - 0.01) * 100) + " %"
                    width: 50
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // Position timeout
            Label { text: qsTr("Position Lost Timeout") }
            Row {
                Slider {
                    id: posTimeoutSlider
                    from: 1
                    to: 60
                    stepSize: 1
                    value: timeBeforePositionLost
                    onValueChanged: timeBeforePositionLost = value
                    width: 200
                }

                Label {
                    text: " " + timeBeforePositionLost + " s"
                    width: 40
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // General data timeout
            Label { text: qsTr("General Data Lost Timeout") }
            Row {
                spacing: 10
                Slider {
                    id: generalDataSlider
                    from: 1
                    to: 60
                    stepSize: 1
                    value: timeBeforeGeneralDataLost
                    onValueChanged: timeBeforeGeneralDataLost = value
                    width: 200
                }
                Label {
                    text: timeBeforeGeneralDataLost + " s"
                    width: 40
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // Boat Trip Line Duration (minutes)
            Label { text: qsTr("Boat Trip Line Duration") }
            Row {
                spacing: 10
                Slider {
                    id: boatLineSlider
                    from: 1        // 1 min
                    to: 60        // 60 min = 1 h
                    stepSize: 1
                    value: distanceLineTimeBoatTrip / 60   // convert seconds to minutes
                    onValueChanged: distanceLineTimeBoatTrip = value * 60   // store as seconds
                    width: 200
                }
                Label {
                    text: Math.round(distanceLineTimeBoatTrip / 60) + " min"
                    width: 50
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // Wind Trip Line Duration (minutes)
            Label { text: qsTr("Wind Trip Line Duration") }
            Row {
                spacing: 10
                Slider {
                    id: windLineSlider
                    from: 1
                    to: 60
                    stepSize: 1
                    value: distanceLineTimeWindTrip / 60
                    onValueChanged: distanceLineTimeWindTrip = value * 60
                    width: 200
                }
                Label {
                    text: Math.round(distanceLineTimeWindTrip / 60) + " min"
                    width: 50
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // Minimum Track Points Distance
            Label { text: qsTr("Minimum Distance Between Track Points") }

            Row {
                spacing: 10

                Slider {
                    id: minTrackDistanceSlider
                    from: 10
                    to: 500
                    stepSize: 5
                    value: minimumTrackPointsDistance
                    onValueChanged: minimumTrackPointsDistance = value
                    width: 200
                }

                Label {
                    text: minimumTrackPointsDistance + " m"
                    width: 50
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // Max track points
            Label { text: qsTr("Maximum Track Points") }
            Row {
                spacing: 10
                Slider {
                    id: maxTrackSlider
                    from: 50
                    to: 5000
                    stepSize: 10
                    value: maxTrackPoints
                    onValueChanged: maxTrackPoints = value
                    width: 200
                }
                Label {
                    text: maxTrackPoints
                    width: 50
                    horizontalAlignment: Text.AlignLeft
                }
            }

        }
    }



    //// Subsubmenus ////
    //Center View
    Menu {
        id: centerViewSubmenu
        width: rightClickMenuWidth/1.2
        modal: true

        MenuItem {
            enabled: boatPositionInit

            contentItem: Label {
                text: qsTr("On Boat (Z)")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }

            onTriggered: {
                goToPositionAnimation(currentBoatCoord, 15)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("On Cursor")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: {
                followBoat = false
                goToPositionAnimation(cursorCoord, 15)
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("On Position")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: centerViewDialog.open()
        }
    }

    //View Up
    Menu {
        id: viewUpSubmenu
        width: rightClickMenuWidth/1.2
        modal: true

        MenuItem {
            contentItem: Label {
                text: qsTr("North Up")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: {
                mapViewMode = 0
                freeViewUp = 0
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Heading Up")
                enabled: boatHeadingReceived
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: {
                mapViewMode = 1
                freeViewUp = boatHeading
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Course Up")
                enabled: boatCourseReceived
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: {
                mapViewMode = 2
                freeViewUp = boatCourse
            }
        }

        MenuItem {
            contentItem: Label {
                text: qsTr("Free Rotation")
                enabled: boatCourseReceived
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width
            }
            onTriggered: mapViewMode = 3
        }
    }



    ///////////////
    /// Markers ///
    ///////////////
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



    ///////////////////////////
    /// Data Labels Overlay ///
    ///////////////////////////

    //// Left Side ////
    Column {
        id: leftSideInfoColumn
        visible: showWidgets

        anchors.top: parent.top
        anchors.topMargin: labelVerticalMargin * 2
        anchors.left: parent.left
        anchors.leftMargin: labelLateralMargin
        spacing: labelVerticalMargin

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
                    case 0: default: return qsTr("North Up")
                    case 1: return qsTr("Heading Up")
                    case 2: return qsTr("Course Up")
                    case 3: return qsTr("Free View") + "\n" +
                               qsTr("Rotation: ") + Math.round(Utils.normalizeAngle180(mapRotation)) + "°"
                }
            }
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
            text: {
                var percent = (map.zoomLevel - map.minimumZoomLevel) / (map.maximumZoomLevel - map.minimumZoomLevel) * 100
                percent = Math.max(0, Math.min(100, percent))
                return (qsTr("Zoom: ") + Math.round(percent) + "%" +"\n" +
                        qsTr("Tilt: ") + Math.round(map.tilt) + "°")
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
            text: qsTr("Cursor Position") + "\n" +
                   qsTr("Lat: ") + Utils.formatLat(cursorLatitude) + "\n" +
                   qsTr("Lon: ") + Utils.formatLon(cursorLongitude)
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
            text: qsTr("From Boat") + "\n" +
                  qsTr("Distance: ") + (Utils.metersToNauticalMiles(cursorDistanceBoat) > 100
                                 ? Utils.metersToNauticalMiles(cursorDistanceBoat).toFixed(0)
                                 : Utils.metersToNauticalMiles(cursorDistanceBoat).toFixed(2)) + "NM" + "\n" +
                  qsTr("Bearing: ") + cursorBearingBoat.toFixed(0) + "°" + "\n" +
                  qsTr("ETA: ") + Utils.secondsToDHMS(Utils.getETA(cursorDistanceBoat, boatSpeed))
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


    //// Right Side ////
    Column {
        id: rightSideInfoColumn
        visible: showWidgets

        anchors.top: parent.top
        anchors.topMargin: labelVerticalMargin * 2
        anchors.right: parent.right
        anchors.rightMargin: labelLateralMargin
        spacing: labelVerticalMargin

        // Boat Date
        Label {
            id: dateLabel
            color: labelColor
            visible: dateReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Date: ") + date
        }

        // Boat Time
        Label {
            id: timeLabel
            color: labelColor
            visible: timeReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Time: ") + time
        }

        // Satellites in view
        Label {
            id: satellitesInViewLabel
            color: labelColor
            visible: satellitesReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Satellites: ") + satellitesInView
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
            text: boatPositionReceived ? qsTr("Boat Position\nLat: ") + Utils.formatLat(boatLatitude) + "\n"+ qsTr("Lon: ") + Utils.formatLon(boatLongitude)
                                       : qsTr("Last Boat Position\nLat: ") + Utils.formatLat(boatLatitude) + "\n" + qsTr("Lon: ") + Utils.formatLon(boatLongitude)
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
            text: qsTr("Heading: ") + boatHeading.toFixed(1) + "°"
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
            text: qsTr("Course: ") + boatCourse.toFixed(1) + "°"
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
            text: qsTr("Speed: ") + boatSpeed.toFixed(1) + qsTr("kts")
        }

        // Wind
        Label {
            id: windLabel
            color: labelColor
            visible: windReceived && boatHeadingReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding

            background: Rectangle {
                color: labelBackgroundColor
                radius: labelBackgroundRadius
            }

            font.pixelSize: labelFontSize

            text: {
                // True wind direction (relative to North)
                var trueDir = (boatHeading + windAngle) % 360
                if (trueDir < 0) trueDir += 360

                return qsTr("True Wind") + "\n"
                     + qsTr("Direction: ")
                     + Math.round(trueDir) + "°\n"
                     + qsTr("Speed: ")
                     + windSpeed.toFixed(1) + "kts"
            }
        }


        // Depth
        Label {
            id: depthLabel
            color: labelColor
            visible: depthReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Depth: ") + depth.toFixed(1) + "m"
        }

        // Water Temperature
        Label {
            id: waterTemperatureLabel
            color: labelColor
            visible: waterTemperatureReceived
            width: labelRightSideWidth
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            padding: labelPadding
            background: Rectangle {
                color: labelBackgroundColor
                radius :  labelBackgroundRadius
            }
            font.pixelSize: labelFontSize
            text: qsTr("Water Temp: ") + waterTemperature.toFixed(1) + "°C"
        }
    }

    Canvas {
        id: compassCanvas

        width: 150
        height: 150
        visible: (showWidgets)

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: labelVerticalMargin
        anchors.rightMargin: labelLateralMargin

        property real heading: boatHeading
        property real course: boatCourse
        property real wind: (heading + windAngle) % 360

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var centerX = width / 2
            var centerY = height / 2
            var radius = Math.min(width, height) / 2 - 10

            // Rotate whole compass
            ctx.save()
            ctx.translate(centerX, centerY)
            ctx.rotate(-mapRotation * Math.PI / 180)
            ctx.translate(-centerX, -centerY)

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
            ctx.fillText(qsTr("NE"), centerX + radius * 0.55, centerY - radius * 0.55);
            ctx.fillText(qsTr("SE"), centerX + radius * 0.55, centerY + radius * 0.55);
            ctx.fillText(qsTr("SW"), centerX - radius * 0.55, centerY + radius * 0.55);
            ctx.fillText(qsTr("NW"), centerX - radius * 0.55, centerY - radius * 0.55);

            // Draw heading arrow
            if(boatHeadingReceived)
            {
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
            }

            // Draw course arrow
            if(boatCourseReceived)
            {
                ctx.save()
                ctx.translate(centerX, centerY)
                ctx.rotate((course - 0) * Math.PI / 180)

                ctx.beginPath()
                ctx.moveTo(0, -radius + 15)
                ctx.lineTo(4, 0)
                ctx.lineTo(-4, 0)
                ctx.closePath()

                ctx.fillStyle = "blue"
                ctx.fill()
                ctx.restore()
            }

            // Draw wind arrow (green)
            if (windReceived)
            {
                ctx.save()
                ctx.translate(centerX, centerY)

                if (wind < 0)
                    wind += 360

                ctx.rotate(wind * Math.PI / 180)

                ctx.beginPath()
                ctx.moveTo(0, -radius + 15)
                ctx.lineTo(2, -5)
                ctx.lineTo(-2, -5)
                ctx.closePath()

                ctx.fillStyle = "green"
                ctx.fill()
                ctx.restore()
            }

            // Draw center black circle
            ctx.beginPath()
            ctx.arc(centerX, centerY, 5, 0, 2 * Math.PI)
            ctx.fillStyle = "black"
            ctx.fill()

            ctx.restore()
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

            function onWindChanged() {
                compassCanvas.requestPaint()
            }

        }
    }


    //// View-Zoom Actions / Bottom Left ////
    Column {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: labelLateralMargin
        anchors.bottomMargin: labelVerticalMargin
        spacing: labelVerticalMargin

        // Scale bar
        Item {
            id: scaleBar
            width: 140
            height: 28

            property int scaleBarPx: 130
            property real metersPerPixel: Utils.metersPerPixel(map.center.latitude, map.zoomLevel)
            property real scaleMeters: metersPerPixel * scaleBarPx

            //Drawing
            Rectangle {
                width: 2
                height: 10
                color: "black"
                anchors.left: parent.left
                anchors.bottom: parent.bottom
            }
            Rectangle {
                width: scaleBar.scaleBarPx
                height: 2
                color: "black"
                anchors.left: parent.left
                anchors.bottom: parent.bottom
            }
            Rectangle {
                width: 2
                height: 10
                color: "black"
                anchors.left: parent.left
                anchors.leftMargin: scaleBar.scaleBarPx - 2
                anchors.bottom: parent.bottom
            }

            // Label
            Text {
                anchors.bottom: parent.top
                anchors.bottomMargin: -25
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 12
                color: "black"
                text: {
                    var distMeters = scaleBar.scaleMeters

                    // meters / kilometers formatting
                    var km = distMeters / 1000
                    var distStr =
                        distMeters < 1000
                            ? Math.round(distMeters) + " m"
                            : (km < 10 ? km.toFixed(1) : km.toFixed(0)) + " km"

                    // nautical miles formatting
                    var nm = Utils.metersToNauticalMiles(distMeters)
                    var nmStr = (nm < 10 ? nm.toFixed(1) : nm.toFixed(0)) + " NM"

                    return distStr + " / " + nmStr
                }
            }
        }

        // Zoom & Follow
        Row {
            spacing: labelVerticalMargin
            visible: showWidgets
            Column {
                spacing: labelVerticalMargin / 2

                Row { //Zoom button
                    spacing: labelLateralMargin

                    Button {
                        text: qsTr("Zoom -")
                        width: 60
                        height: 30
                        onClicked: mapGoToZoomLevel(map.zoomLevel - 1)
                    }

                    Button {
                        text: qsTr("Zoom +")
                        width: 60
                        height: 30
                        onClicked: mapGoToZoomLevel(map.zoomLevel + 1)
                    }
                }

                Slider { //Zoom slider
                    id: zoomSlider
                    from: map.minimumZoomLevel
                    to: map.maximumZoomLevel
                    stepSize: 0.1
                    value: map.zoomLevel
                    width: 130
                    onValueChanged: mapGoToZoomLevel(value)
                }
            }

            Button { //Follow Boat
                width: 60
                height: 60
                anchors.verticalCenter: parent.verticalCenter
                enabled: boatPositionReceived
                text: (followBoat ? qsTr("Unfollow Boat") : qsTr("Follow Boat")).replace(" ", "\n")
                onClicked: followBoat = !followBoat
            }
        }
 }



    //////////////////
    /// Animations ///
    //////////////////

    //Pan + Zoom On Coordinates
    Item {
        id: panZoomAnimationItem
        property int zoomAnimationDuration: 2000
        property int panAnimationationDuration: 2000

        PropertyAnimation{
            id: fullAnimation
            target: map
            duration: panZoomAnimationItem.zoomAnimationDuration
            easing.type: Easing.OutExpo
            onStarted: panAnimation.start()
            onFinished: zoomAnimation.start()
        }

        PropertyAnimation{
            id: panAnimation
            target: map
            property: "center"
            to: panZoomAnimCoord
            duration: panZoomAnimationItem.panAnimationationDuration
            easing.type: Easing.InOutExpo
        }

        PropertyAnimation{
            id: zoomAnimation
            target: map
            property: "zoomLevel"
            duration: panZoomAnimationItem.zoomAnimationDuration
            easing.type: Easing.InExpo
        }
    }



    //////////////////
    /// Update Map ///
    //////////////////

    //Update map center on new position
    function centerMapOnPosition(coord) {
        map.center = coord
    }

    function centerMapOnPositionSmooth(coord) {
        if (panAnimation.running || fullAnimation.running)
            return

        panZoomAnimCoord = coord
        panAnimation.start()
    }

    //Go To Zoom Level Map
    function mapGoToZoomLevel(zoomLevel) {
        if(zoomLevel > map.maximumZoomLevel)
            zoomLevel = map.maximumZoomLevel
        else if(zoomLevel < map.minimumZoomLevel)
            zoomLevel = map.minimumZoomLevel

        map.zoomLevel = zoomLevel
    }

    function mapGoToZoomLevelSmooth(zoomLevel) {
        if (zoomAnimation.running || fullAnimation.running)
            return

        if(zoomLevel > map.maximumZoomLevel)
            zoomLevel = map.maximumZoomLevel
        else if(zoomLevel < map.minimumZoomLevel)
            zoomLevel = map.minimumZoomLevel

        zoomAnimation.to = zoomLevel
        zoomAnimation.start()
    }

    //PanZoom Animation On Coordinates
    function goToPositionAnimation(coord, zoomLevel) {
        if (panAnimation.running || zoomAnimation.running || fullAnimation.running)
            return

        panZoomAnimCoord = coord
        panAnimation.start()

        zoomAnimation.to = zoomLevel
        zoomAnimation.start()
    }

    //Add marker
    function addMarkerOnMap(lat, lon) {
        var item = redMarkerImg.createObject(window, {
            coordinate: QtPositioning.coordinate(lat, lon),
            objectName: "marker"
        });

        map.addMapItem(item)
        userMarkerCount++;
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

    //Redraw boat icon
    function updateBoatIconOnMap() {
        // Remove previous boat icon if it exists
        if (boatMarkerRef !== null) {
            map.removeMapItem(boatMarkerRef)
            boatMarkerRef.destroy()
            boatMarkerRef = null
        }

        // Create and store new boat marker
        boatMarkerRef = boatMarkerImg.createObject(window, {
            coordinate: QtPositioning.coordinate(boatLatitude, boatLongitude),
            rotation: boatHeading - mapRotation
        })

        map.addMapItem(boatMarkerRef)
    }

    //Recalculate boat distance/bearing relative to cursor position
    function updateBoatCursorCalculations() {
        cursorDistanceBoat = Utils.haversineDistance(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
        cursorBearingBoat = Utils.calculateBearing(boatLatitude, boatLongitude, cursorLatitude, cursorLongitude)
    }

    //Boat Track
    function drawBoatTrack() {
        //Do not add if track not enabled
        if (!enableTrack)
            return

        //Check if minimum distance between 2 points  before adding
        if (boatTrack.length &&
            Utils.haversineDistance(
                boatTrack[boatTrack.length - 1].latitude,
                boatTrack[boatTrack.length - 1].longitude,
                boatLatitude, boatLongitude
            ) < minimumTrackPointsDistance)
            return

        var updatedTrack = boatTrack.slice()
        updatedTrack.push(QtPositioning.coordinate(boatLatitude, boatLongitude))

        //Erase first waypoint if array exceeds max size
        if (updatedTrack.length > maxTrackPoints)
            updatedTrack.shift()

        boatTrack = updatedTrack   //trigger redrawing
    }



    //////////////
    /// Timers ///
    //////////////
    //Update boat icon on map
    Timer {
        id: updateMapViewOntimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (followBoat){
                centerMapOnPosition(currentBoatCoord)
            }
        }
    }

    //Update compass
    Timer {
        id: updateCompass
        interval: 1000
        running: true
        repeat: true

        onTriggered: compassCanvas.requestPaint()
    }

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
                dateReceived = false
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
                timeReceived = false
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
                    textTimerPositionUpdate = qsTr("Position Updated\n < 1s ago")
                else
                    textTimerPositionUpdate = qsTr("Position Updated\n") + Math.ceil(elapsedSec) + qsTr("s ago")
            else{
                boatPositionReceived = false
                textTimerPositionUpdate = qsTr("Position Lost")
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
                depthReceived = false
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
                waterTemperatureReceived = false
        }
    }

    //Satellites in view
    Timer {
        id: updateLastSatellitesTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastSatellites === 0)
                return

            elapsedSec = (Date.now() - timeLastSatellites) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                satellitesReceived = false
        }
    }

    //Boat Wind
    Timer {
        id: updateLastWindTimer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timeLastWind === 0)
                return

            elapsedSec = (Date.now() - timeLastWind) / 1000
            if(elapsedSec > timeBeforeGeneralDataLost)
                windReceived = false
        }
    }



    /////////////////////////////////////////
    /// Update Data From External Signals ///
    /////////////////////////////////////////
    //Update boat UTC time
    function updateTime(newTime) {
        time = newTime

        timeLastUtcTime = Date.now()
        timeReceived = true
    }

    //Update boat UTC Date
    function updateDate(newDate) {
        date = newDate

        timeLastUtcDate = Date.now()
        dateReceived = true
    }

    //Update boat position
    function updateBoatPosition(lat, lon) {
        boatLatitude = lat
        boatLongitude = lon
        currentBoatCoord = QtPositioning.coordinate(lat, lon)

        //Zoom & center on boat first time receiving position
        if(!boatPositionInit){
            followBoat = true
            goToPositionAnimation(currentBoatCoord, 17)
        }

        timeLastPosition = Date.now()
        boatPositionReceived = true
        boatPositionInit = true

        //Draw new boat position
        updateBoatIconOnMap()
        updateBoatCursorCalculations()

        //Add new position to track line
        drawBoatTrack()
    }

    //Update boat heading
    function updateBoatHeading(head) {
        boatHeading = head

        timeLastHeading= Date.now()
        boatHeadingReceived = true
    }

    //Update boat depth
    function updateDepth(newDepth) {
        depth = newDepth

        timeLastDepth = Date.now()
        depthReceived = true
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
    function updateWaterTemperature(temp) {
        waterTemperature = temp

        timeLastWaterTemp = Date.now()
        waterTemperatureReceived = true
    }

    //Update number of satellites in view
    function updateSatellitesInView(satellites) {
        satellitesInView = satellites

        timeLastSatellites = Date.now()
        satellitesReceived = true
    }

    //Update boat relative wind
    function updateWind(angle, speed){
        windAngle = angle
        windSpeed = speed

        timeLastWind = Date.now()
        windReceived = true;
    }

}
