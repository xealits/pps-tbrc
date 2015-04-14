var listener_id, connection;
var connect_button, disconnect_button, get_listeners_button;
var socket_id, console_log;

function enable_connected_buttons() {
  console.log("Connected");
  connect_button.disabled = true;
  disconnect_button.disabled = false;
  get_listeners_button.disabled = false;
  console_log.value = "";
}

function disable_connected_buttons() {
  console.log("Disconnected");
  connect_button.disabled = false;
  disconnect_button.disabled = true;
  get_listeners_button.disabled = true;
  output.innerHTML = "";
}

function init() {
  listener_id = -1;
  connection = 0;
  connect_button = document.getElementById("connect_button");
  disconnect_button = document.getElementById("disconnect_button");
  get_listeners_button = document.getElementById("get_listeners_button");
  socket_id = document.getElementById("socket_id");
  console_log = document.getElementById("console_log");
  
  disable_connected_buttons();
}

function bind_socket() {
  if (connection!==0) {
    console.log("Another attempt was made to connect! Aborting.");
    return;
  }    
  connection = new WebSocket('ws://localhost:1987');
  connection.onerror = function (event) {
    if (event.originalTarget.readyState!==1) {
      console_log.value = "Server not ready ("+event.originalTarget.readyState+") for connection!";
      socket_id.style.backgroundColor = "red";
      connection = 0;
      socket_id = -1;
      disable_connected_buttons();
      //return;
    }
    //console.log("Server"+event.originalTarget.readyState);
  }
  connection.onopen = function () {
    connection.onmessage = function(event) {
      console_log.value = event.data;
      var d = event.data;
      if (d.indexOf("SET_LISTENER_ID")>-1) {
        listener_id = parseInt(d.substr(d.indexOf(":")+1));
        socket_id.value = listener_id;
        socket_id.style.backgroundColor = "lightgreen";
        enable_connected_buttons();
        get_socket_listeners();
        return;
      }
    };
  };
}
  
function handle_disconnect() {
  connection.send("REMOVE_LISTENER:"+listener_id);
  connection.onmessage = function(event) {
    var d = event.data;
    if (d.indexOf("LISTENER_DELETED")>-1) {
      socket_id.value = "###";
      socket_id.style.backgroundColor = "yellow";
      disable_connected_buttons();
      delete connection;
      connection = 0;
      listener_id = -1;
    }
  }
}
function unbind_socket() {
  handle_disconnect();
}

function close() {
  alert("close() invoked");
  handle_disconnect();
  connection.onclose = function (event) {
    console.log(event);
  };
}

function get_socket_listeners() {
  if (connection===0) return;
  if (listener_id<0) return;
  
  connection.send("WEB_GET_LISTENERS:"+listener_id);
  connection.onmessage = function(event) {
    var d = event.data;
    if (d.indexOf("LISTENERS_LIST")>-1) {
      var listeners = d.substr(d.indexOf(":")+1);
      var list = listeners.split(';');
      //console_log.value = listeners;
      var l;
      output.innerHTML = "";
      for (var i=0; i<list.length; i++) {
        var fields = list[i].split(',');
        if (fields.length<3) continue;
        var pre = document.createElement("span");
        pre.setAttribute('id', 'button_'+fields[0]);
        //pre.setAttribute('width', '350px');
        //pre.setAttribute('border', '1px solid black');
        pre.innerHTML = fields[1];
        pre.style.backgroundColor = (parseInt(fields[2])) ? "yellow" : "lightgreen";
        output.appendChild(pre);
      }
    }
  };
}
