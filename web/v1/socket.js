var port = 1987;
var listener_id, connection;
var upper_fields;
var bind_button, unbind_button, refresh_button, acquisition_button, time_field;
var socket_id, run_id, console_log, dqm_block;
var built_clients;
var acquisition_started;

Array.prototype.diff = function(b) {
  return this.filter(function(i) { return b.indexOf(i)<0; });
}

String.prototype.has_key = function(key_str) {
  if (this.indexOf(key_str)>-1) return true;
  return false;
}

String.prototype.value = function() {
  return this.substr(this.indexOf(":")+1);
}

function enable_connected_buttons() {
  console.log("Connected");
  socket_id.style.backgroundColor = "lightgreen";
  bind_button.disabled = true;
  unbind_button.disabled = false;
  refresh_button.disabled = false;
  acquisition_button.disabled = false;
  console_log.value = "";
}

function disable_connected_buttons() {
  console.log("Disconnected");
  bind_button.disabled = false;
  unbind_button.disabled = true;
  refresh_button.disabled = true;
  acquisition_button.disabled = true;
  output.innerHTML = "";
  socket_id.value = "###";
}

function restore_init_state() {
  socket_id.value = "###";
  run_id.value = "###";
  socket_id.style.backgroundColor = "yellow";
  disable_connected_buttons();
  listener_id = -1;
  time_field.style.color = "lightgray";
  if (connection!=0) {
    connection.close();
    delete connection;
    connection = 0;
  }
}

function socket_init() {
  listener_id = -1;
  connection = 0;
  
  if (!window.WebSocket) alert("Your browser does not support WebSockets!");

  upper_fields = document.getElementById("upper_fields");
  bind_button = document.getElementById("bind_button");
  unbind_button = document.getElementById("unbind_button");
  refresh_button = document.getElementById("refresh_button");
  acquisition_button = document.getElementById("acquisition_button");
  socket_id = document.getElementById("socket_id");
  run_id = document.getElementById("run_id");
  console_log = document.getElementById("console_log");
  time_field = document.getElementById("time_field");
  exception_block = document.getElementById("exception_block");
  dqm_block = document.getElementById("dqm_block");
  
  restore_init_state();
  disable_connected_buttons();
}

function bind_socket() {
  if (connection!==0) {
    console.error("Wrong (or multiple) attempt made to bind! Aborting.");
    connection = 0;
    return;
  }

  hostname = 'localhost';
  //hostname = '137.138.105.83';
  connection = new WebSocket('ws://'+hostname+':'+port);

  bind_button.disabled = true;
  built_clients = [];

  connection.onerror = function (event) {
    if (event.originalTarget===undefined || event.originalTarget.readyState!==1) {
      console_log.value = "Server OOS!";
      socket_id.style.backgroundColor = "red";
      socket_id.value = -1;
      disable_connected_buttons();
      return;
    }
    //unbind_socket();
  }
  connection.onopen = function () { };
  connection.onmessage = function(event) { parse_message(event); };
  connection.onclose = function() {
    connection = 0;
    bind_socket();
  };
}
  
function ask_socket_removal(id) {
  if (connection===0) return;
  connection.send("REMOVE_CLIENT:"+id);
  connection.onmessage = function(event) { parse_message(event); }  
}

function unbind_socket() {
  ask_socket_removal(listener_id);
}

function ask_socket_ping(id) {
  if (connection===0) return;
  connection.send("PING_CLIENT:"+id);
  connection.onmessage = function(event) { parse_message(event); }  
}

function socket_close() {
  if (connection===0) return;
  unbind_socket();
  connection.onclose = function (event) {
    console_log.value = "Connection to ppsRun successfully closed";
  };
  connection.close();
  //bind_socket();
}

function get_run_id() {
  if (connection===0) return;
  connection.send("GET_RUN_NUMBER:"+listener_id);
  connection.onmessage = function(event) { parse_message(event); };
}

function start_acquisition() {
  if (connection===0) return;
  get_run_id();
  connection.send("START_ACQUISITION:"+listener_id);
  connection.onmessage = function(event) { parse_message(event); }
}

function stop_acquisition() {
  connection.send("STOP_ACQUISITION:"+listener_id);
  connection.onmessage = function(event) { parse_message(event); }  
}

function parse_message(event) {
  var d = event.data.replace('\0', '');
  
  if (d.has_key("SET_CLIENT_ID")) {
    listener_id = parseInt(d.value());
    socket_id.value = listener_id;
    enable_connected_buttons();
    socket_refresh();
    console_log.value += d+'\n';
  }
  else if (d.has_key("CLIENTS_LIST")) {
    var listeners = d.value();
    var list = listeners.split(';');
    var retrieved = [];
    var retrieved_ids = [];
    var num_det = 0;
    for (var i=0; i<list.length; i++) {
      var fields = list[i].split(',');
      if (fields.length<3) continue;
      var id = parseInt(fields[0]);
      if (id===listener_id) continue;
      var name = fields[1];
      var socket_type = parseInt(fields[2]);
      retrieved.push({'id': id, 'name': name, 'type': socket_type});
      retrieved_ids.push(id);
      if (socket_type===3) num_det += 1;
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
    if (num_det===0) { // no detector process has been found
      acquisition_button.innerHTML = "Start acquisition";
      acquisition_button.setAttribute('onClick', 'start_acquisition()');
      acquisition_started = false;
    }
    else {
      acquisition_button.innerHTML = "Stop acquisition";
      acquisition_button.setAttribute('onClick', 'stop_acquisition()');
      acquisition_started = true;
    }
  }
  else if (d.has_key("THIS_CLIENT_DELETED")) {
    restore_init_state();
    unbind_socket();
  }
  else if (d.has_key("OTHER_CLIENT_DELETED")) {
    var id = parseInt(d.value());
    if (id===listener_id) { restore_init_state(); unbind_socket(); }
    else console.log("Socket "+id+" successfully deleted!");
    console_log.value += d+'\n';
  }
  else if (d.has_key("PING_ANSWER")) {
    alert(d.value());
    console_log.value += d+'\n';
  }
  else if (d.has_key("ACQUISITION_STARTED")) {
    //alert("Acquisition process successfully launched!");
    acquisition_button.innerHTML = "Stop acquisition";
    acquisition_button.setAttribute('onClick', 'stop_acquisition()');
    acquisition_started = true;
  }
  else if (d.has_key("ACQUISITION_STOPPED")) {
    //alert("Acquisition process terminated!");
    acquisition_button.innerHTML = "Start acquisition";
    acquisition_button.setAttribute('onClick', 'start_acquisition()');
    acquisition_started = false;
  }
  else if (d.has_key("NEW_DQM_PLOT")) {
    add_dqm_plot(d.value());
  }
  else if (d.has_key("UPDATED_DQM_PLOT")) {
    var existing_img = document.getElementById(d.value());
    if (existing_img===null) add_dqm_plot(d.value());
    else {
      var img = existing_img.getElementByTagName("img");
alert(img.src);
      img.setAttribute('src', 'file:///tmp/'+d.value()+".png?"+new Date().getTime());
      //img.src = 'file:///tmp/'+d.value()+".png?"+new Date().getTime();
alert(img.src);
    }
  }
  else if (d.has_key("RUN_NUMBER")) {
    run_id.value = parseInt(d.value());
  }
  else if (d.has_key("EXCEPTION")) {
    var rx = /^\[(.*)\] === (.*)\ === (.*)/g;
    var extr = rx.exec(d.value());
    exception_block.innerHTML += "<small>"+(new Date).toLocaleTimeString()+"</small>&nbsp;";
    var message = document.createElement("small");
    message.className = "exception-message";
    switch (parseInt(extr[1])) {
      case  0: message.style.color = "darkorange"; break; // Info
      case  1: message.style.color = "dodgerblue"; break; // JustWarning
      case  2: message.style.color = "red"; break;        // Fatal
      case -1: default: message.style.color = "black"; break; // Undefined
    }
    message.innerHTML += extr[3];
    var tooltip = document.createElement("div");
    tooltip.className = "tooltip";
    tooltip.innerHTML = "<b>"+extr[2]+"</b>";
    message.appendChild(tooltip);
    exception_block.appendChild(message);
    exception_block.innerHTML += "<br />";
    console.log(d);
  }
  else if (d.has_key("MASTER_DISCONNECT")) {
    alert("ALERT:\nMaster disconnected!");
    restore_init_state();
    return;
  }  
}

function socket_refresh() {
  if (connection===0) return;
  else if (connection===undefined) return;
  if (listener_id<0) return;
  
  connection.send("WEB_GET_CLIENTS:"+listener_id);
  connection.onmessage = function(event) { parse_message(event); };

  time_field.style.color = "black";
  time_field.innerHTML = new Date;
}

function add_dqm_plot(value) {
  var img = document.createElement("img");
  img.setAttribute('src', 'file:///tmp/'+value+".png");
  img.setAttribute('alt', value);
  img.setAttribute('width', "200");
  var link = document.createElement("a");
  link.setAttribute('href', 'file:///tmp/'+value+".png");
  link.setAttribute('target', "_blank");
  link.setAttribute('id', value);
  link.appendChild(img);
  dqm_block.insertBefore(link, dqm_block.childNodes[0]);
  console.log(d.value());
}

function create_block(obj) {  
  var block = document.createElement("span");
  block.setAttribute('id', 'socket_block_'+obj.id);
  block.className = "socket_block";
  
  var title = document.createElement("div");
  title.innerHTML = obj.name;
  title.className = "socket_block_title";
  block.appendChild(title);
  
  var button_close = document.createElement("button");
  button_close.setAttribute('id', 'close_socket_'+obj.id);
  button_close.setAttribute('onclick', 'ask_socket_removal('+obj.id+')');
  button_close.innerHTML = "Close";
  block.appendChild(button_close);
  
  /*var button_ping = document.createElement("button");
  button_ping.setAttribute('id', 'ping_socket_'+obj.id);
  button_ping.setAttribute('onclick', 'ask_socket_ping('+obj.id+')');
  button_ping.innerHTML = "Ping";
  block.appendChild(button_ping);*/
  
  switch (obj.type) {
    case 0: // master socket
      block.className += ' block_master';
      button_close.disabled = true;
      //button_ping.disabled = true;
      break;
    case 1: // web socket
      block.className += ' block_websocket';
      //button_ping.disabled = true;
      break;
    case 2: // regular socket
      block.className += ' block_regularsocket';
      break;
    case 3: // detector socket
      block.className += ' block_detsocket';
      button_close.disabled = true;
      //button_ping.disabled = true;
      acquisition_button.innerHTML = "Stop acquisition";
      acquisition_button.setAttribute('onClick', 'stop_acquisition()');
      acquisition_started = true;
      dqm_block.disabled = false;
      break;
    case 4: // DQM socket
      block.className += ' block_dqmsocket';
      break;
    default:
      block.className += ' block_othersocket';
      break;
  }
  
  return block;
}

function remove_block(id) {
  return (elem=document.getElementById("socket_block_"+id)).parentNode.removeChild(elem);
}

