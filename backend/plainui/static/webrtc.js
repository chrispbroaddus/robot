const VEHICLE_ID = 'r01';
const CAMERA_ID = 'front';
const USER_ID = 'u01';
const CONNECTION_ID = '';

var videoElem;  // the video HTML element
var peerConnection;  // the webrtc connection
var serverConnection;  // the websocket connection

// generate uuidv4 function 
function uuidv4 () {
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
    var r = Math.random() * 16 | 0
    var v = c == 'x' ? r : (r & 0x3 | 0x8)
    return v.toString(16)
  })
}

CONNECTION_ID = uuidv4()

// configuration for the webrtc handshake
var webrtcConfig = {
    'iceServers': [
        {'urls': 'stun:stun.services.mozilla.com'},
        {'urls': 'stun:stun.l.google.com:19302'},
    ]
};

function pageReady() {
    videoElem = document.getElementById('video');

    url = 'ws://localhost:8000/api/v1/ws/vehicle/' + VEHICLE_ID + '/user/' + USER_ID + '/subscribe';
    console.log(url);
    serverConnection = new WebSocket(url);
    serverConnection.onmessage = handleWebsocketMessage;
    serverConnection.onopen = handleWebsocketOpen;
}

function requestCamera(cameraID) {
    peerConnection = new RTCPeerConnection(webrtcConfig);
    peerConnection.onicecandidate = handleIceCandidate;
    peerConnection.onaddstream = handleRemoteStream;

    serverConnection.send(JSON.stringify({
        videoRequest: {
            camera: cameraID,
            connectionID: CONNECTION_ID
        }
    }));
}

function handleWebsocketOpen() {
    console.log("at handleWebsocketOpen");
    // kick off the handshake to establish a webrtc stream
    requestCamera(CAMERA_ID);
}

function handleWebsocketMessage(message) {
    var signal = JSON.parse(message.data);

    if (signal.sdpRequest) {
        var offer = signal.sdpRequest;
        console.log("received offer:", signal.sdpRequest);
        $("#remote").val(signal.sdpRequest.sdp);
        var offer = new RTCSessionDescription({
            type: "offer",
            sdp: signal.sdpRequest.sdp,
        });
        peerConnection.setRemoteDescription(offer).then(function() {
            // operator is always the answerer
            peerConnection.createAnswer().then(handleDescriptionCreated).catch(handleError);
        }).catch(handleError);
    } else if (signal.iceCandidate) {
        var cand = signal.iceCandidate;
        console.log("got remote ICE candidate:", signal.iceCandidate);
        var cand = new RTCIceCandidate({
            candidate: signal.iceCandidate.candidate,
            sdpMid: signal.iceCandidate.sdp_mid,
            sdpMLineIndex: signal.iceCandidate.sdp_mline_index,
        });
        peerConnection.addIceCandidate(cand).catch(handleError);
    } else if (signal.sdpConfirmation) {
        console.log("we're connected to the vehicle now")
    }
}

function handleIceCandidate(event) {
    console.log("at handleIceCandidate:", event.candidate);
    if (event.candidate != null) {
        serverConnection.send(JSON.stringify({
        iceCandidate: {
          connectionID: CONNECTION_ID,
          candidate: event.candidate.candidate,
          sdpMid: event.candidate.sdpMid,
          sdpMlineIndex: event.candidate.sdpMlineIndex
        }
        }));
    }
}

function handleDescriptionCreated(description) {
    console.log("at handleDescriptionCreated:", description);
    peerConnection.setLocalDescription(description).then(function() {
        console.log("sending SDP to backend:", peerConnection.localDescription);

        $("#local").val(peerConnection.localDescription.sdp);
        serverConnection.send(JSON.stringify({
            sdpRequest: {
                sdp: peerConnection.localDescription.sdp,
                connectionID: CONNECTION_ID
            }
        }));
    }).catch(handleError);
}

function handleRemoteStream(event) {
    console.log('webrtc stream now connected');
    videoElem.srcObject = event.stream;

    // let the server know you're connected
    serverConnection.send(JSON.stringify({
        connected: true,
        connectionID: CONNECTION_ID
    }));
}

function handleError(error) {
    console.log(error);
}
