var listener_id, connection;
var bind_button, unbind_button, refresh_button, time_field;
var socket_id, console_log;
var built_clients;

Array.prototype.diff = function(b) {
  /*var a = this;
  var onlyInA = a.filter(function(curr){ return b.filter(function(cb){ return cb.value==curr.value && cb.display==curr.display }).length===0 });
  var onlyInB = b.filter(function(curr){ return a.filter(function(ca){ return ca.value==curr.value && ca.display==curr.display }).length===0 });
  return onlyInA.concat(onlyInB);*/
  return this.filter(function(i) { return b.indexOf(i)<0; });
}

function enable_connected_buttons() {
  console.log("Connected");
  bind_button.disabled = true;
  unbind_button.disabled = false;
  refresh_button.disabled = false;
  console_log.value = "";
}

function disable_connected_buttons() {
  console.log("Disconnected");
  bind_button.disabled = false;
  unbind_button.disabled = true;
  refresh_button.disabled = true;
  output.innerHTML = "";
  socket_id.value = "###";
}

function restore_init_state() {
  socket_id.value = "###";
  socket_id.style.backgroundColor = "yellow";
  disable_connected_buttons();
  listener_id = -1;
  if (connection!=0) {
    delete connection;
    connection = 0;
  }
}

function socket_init() {
  listener_id = -1;
  connection = 0;
  bind_button = document.getElementById("bind_button");
  unbind_button = document.getElementById("unbind_button");
  refresh_button = document.getElementById("refresh_button");
  socket_id = document.getElementById("socket_id");
  console_log = document.getElementById("console_log");
  time_field = document.getElementById("time_field");
  
  restore_init_state();
  disable_connected_buttons();
}

function bind_socket() {
  if (connection!==0) {
    console.log("Another attempt was made to bind! Aborting.");
    return;
  }
  connection = new WebSocket('ws://localhost:1987');

  bind_button.disabled = true;
  built_clients = [];

  connection.onerror = function (event) {
    if (event.originalTarget.readyState!==1) {
      console_log.value = "Server not ready ("+event.originalTarget.readyState+") for connection!";
      socket_id.style.backgroundColor = "red";
      connection = 0;
      socket_id.value = -1;
      bind_button.disabled = false;
      disable_connected_buttons();
      return;
    }
    unbind_socket();
    //console.log("Server"+event.originalTarget.readyState);
  }
  connection.onopen = function () {
    connection.onmessage = function(event) {
      parse_message(event);
    };
  };
  connection.onclose = function() { 
    bind_socket();
    //socket_close();
    //handle_disconnect();
  };
}
  
function unbind_socket() {
  if (connection===0) return;
  
  connection.send("REMOVE_LISTENER:"+listener_id);
  connection.onmessage = function(event) {
    parse_message(event);
    console.log(event);
  }
}

function socket_close() {
  if (connection===0) return;

  console_log.value = "close() invoked";
  unbind_socket();
  connection.onclose = function (event) {
    console.log(event);
  };
  connection.close();
}

function parse_message(event) {
  console_log.value = event.data;
  var d = event.data;
  if (d.indexOf("SET_LISTENER_ID")>-1) {
    listener_id = parseInt(d.substr(d.indexOf(":")+1));
    socket_id.value = listener_id;
    socket_id.style.backgroundColor = "lightgreen";
    enable_connected_buttons();
    socket_refresh();
  }
  else if (d.indexOf("LISTENERS_LIST")>-1) {
    var listeners = d.substr(d.indexOf(":")+1);
    var list = listeners.split(';');
    var retrieved = [];
    var retrieved_ids = [];
    for (var i=0; i<list.length; i++) {
      var fields = list[i].split(',');
      if (fields.length<3) continue;
      var id = parseInt(fields[0]);
      var name = fields[1];
      var socket_type = parseInt(fields[2]);
      retrieved.push({'id': id, 'name': name, 'type': socket_type});
      retrieved_ids.push(id);
    }
    var difference = retrieved_ids.diff(built_clients);
    for (var i=0; i<difference.length; i++) {
      var idx = retrieved_ids.indexOf(difference[i]);
      output.appendChild(create_block(retrieved[idx]));
      built_clients.push(retrieved_ids[idx]);
    }
    difference = built_clients.diff(retrieved_ids);
    for (var i=0; i<difference.length; i++) {
      remove_block(difference[i]);
      built_clients.splice(built_clients.indexOf(difference[i]), 1);
    }
  }
  else if (d.indexOf("LISTENER_DELETED")>-1) {
    //connection = 0;
    //bind_socket();
    restore_init_state();
    unbind_socket();
  }
  else if (d.indexOf("MASTER_DISCONNECT")>-1) {
    alert("ALERT:\nMaster disconnected!");
    connection.close();
    restore_init_state();
    return;
  }  
}

function socket_refresh() {
  if (connection===0 || connection===undefined) return;
  if (listener_id<0) return;
  
  connection.send("WEB_GET_LISTENERS:"+listener_id);
  connection.onmessage = function(event) {
    parse_message(event);
  };
  time_field.innerHTML = new Date;
}

function create_block(obj) {  
  var block = document.createElement("button");
  
  block.setAttribute('id', 'socket_block_'+obj.id);
  block.className = "socket_block";
  block.innerHTML = "<b>"+obj.name+"<b>";
  
  if (obj.id===listener_id) { // this is us
    block.style.backgroundColor = "lightblue";
    block.disabled = true;
    block.innerHTML += "\n[me]";
  }
  else if (obj.type===1) block.style.backgroundColor = "yellow"; // web socket
  else block.style.backgroundColor = "lightgreen"; // regular socket
  
  return block;
}

function remove_block(id) {
  return (elem=document.getElementById("socket_block_"+id)).parentNode.removeChild(elem);
}

