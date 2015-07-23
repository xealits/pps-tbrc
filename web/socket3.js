var port = 1987;
var listener_id, connection = undefined;
var upper_fields;
var bind_button, unbind_button, refresh_button, acquisition_button, time_field;
var socket_id, console_log;
var built_clients;
var acquisition_started;




function parse_message(event) {
  var d = event.data.slice(0,-1);
  // console_log.value = d;
  // append_to_console( "<p>" + d + "</p>" );
  
  if (d.indexOf("SET_CLIENT_ID")>-1) {
    listener_id = parseInt(d.substr(d.indexOf(":")+1));
    socket_id.value = listener_id;
    enable_connected_buttons();
    socket_refresh();
  }
  else if (d.indexOf("CLIENTS_LIST")>-1) {
    var listeners = d.substr(d.indexOf(":")+1);
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
  else if (d.indexOf("THIS_CLIENT_DELETED")>-1) {
    restore_init_state();
    unbind_socket();
  }
  else if (d.indexOf("OTHER_CLIENT_DELETED")>-1) {
    var id = parseInt(d.substr(d.indexOf(":")+1));
    if (id===listener_id) {
      restore_init_state();
      unbind_socket();
    }
    else console.log("Socket "+id+" successfully deleted!");
  }
  else if (d.indexOf("PING_ANSWER")>-1) {
    alert(d.substr(d.indexOf(":")+1));
  }
  else if (d.indexOf("ACQUISITION_STARTED")>-1) {
    alert("Acquisition process successfully launched!");
    acquisition_button.innerHTML = "Stop acquisition";
    acquisition_button.setAttribute('onClick', 'stop_acquisition()');
    acquisition_started = true;
  }
  else if (d.indexOf("ACQUISITION_STOPPED")>-1) {
    alert("Acquisition process terminated!");
    acquisition_button.innerHTML = "Start acquisition";
    acquisition_button.setAttribute('onClick', 'start_acquisition()');
    acquisition_started = false;
  }
  else if (d.indexOf("EXCEPTION")>-1) {
    console_log.innerHTML = d;
    console.log(d);
  }
  else if (d.indexOf("MASTER_DISCONNECT")>-1) {
    alert("ALERT:\nMaster disconnected!");
    restore_init_state();
    return;
  }  
};





function append_to_console( html ) {
  $( "#console_log" ).append( html );
  var psconsole = $('#console_log');
  if(psconsole.length)
    psconsole.scrollTop(psconsole[0].scrollHeight - psconsole.height());
}


function send_message( con, message ){
  con.send( message );
  append_to_console( "<p>Sent: " + message +"<\p>" );
}


function unbind_socket() {
  window.clearInterval( image_changer );
  if (connection==undefined) return;
  send_message(connection, "REMOVE_CLIENT:" + listener_id);
  // connection.send("REMOVE_CLIENT:"+listener_id);
  connection.close();
  connection = undefined;
  // connection = undefined;
  // interface_off();
  // connection.onmessage = function(event) { parse_message(event); }
}


function socket_refresh() {
  if (connection===0 || connection===undefined) return;
  if (listener_id<0) return;
  
  // connection.send("WEB_GET_CLIENTS:"+listener_id);
  send_message(connection, "WEB_GET_CLIENTS:" + listener_id);
  // connection.onmessage = function(event) {
    // parse_message(event);
  // };
  bind_time.style.color = "black";
  bind_time.innerHTML = new Date();
}


function start_acquisition() {
  if (connection===0) alert("no connection initiated");
  send_message(connection, "START_ACQUISITION:" + listener_id );
  // connection.send("START_ACQUISITION:"+listener_id);
  // connection.onmessage = function(event) { parse_message(event); } // CHECK: why use it? we set the handler on creation of the connection
}





function bind_socket() {
  connection_uri = $("#connection_uri").val();
  // hostname = $("#hostname").val();

  // alert( connection );
  // alert( hostname + ":" + port );
  // if (connection==0) { alert("no connection") };

  if (connection!==undefined) {
    alert("repeated connection");
    console.error("Wrong (or multiple) attempt made to bind! Aborting.");
    // connection = 0;
    return;
  }

  // hostname = 'localhost';
  //hostname = '137.138.105.83';
  connection = new WebSocket(connection_uri);

  // alert( connection );
  // alert( connection );

  // bind_button.disabled = true;
  // built_clients = [];

    connection.onopen = function () {
      // what a hell
      append_to_console( '<p class="incoming">GOT: Connection opened<\p>' );
      interface_on();
      // alert("socket onopen");
    };
    connection.onerror = function (event) {
      // if (event.originalTarget===undefined || event.originalTarget.readyState!==1) {
      //   // $( "#console_log" ).append( "<p>Server not ready for connection!<\p>");
      //   append_to_console( "<p>Server not ready for connection!<\p>" );
      //   $( "#socket_id" ).css("backgroundColor", "red");
      //   $( "#socket_id" ).val( -1 );
      //   // connection = undefined;
      //   interface_off();
      //   return;
      // }
      // unbind_socket();
      alert("socket onerror");
      // connection.close();
      connection = undefined;
      alert( connection );
      interface_off();
  };
  connection.onmessage = function(event) {
    append_to_console( '<p class="incoming">GOT: ' + event.data + "<\p>" );
    parse_message(event);
    // $( "#console_log" ).append( "<p>GOT: " + event.data + "<\p>" );
    // alert( "socket onmessage" );
  }; // TODO
  connection.onclose = function() {
      // bind_socket();
      // Disable interface and remove connection object
      // connection.close();
      connection = undefined;
      append_to_console( '<p> Removed connection <\p>' );
      // alert( "connection closed" );
      // alert( connection );
      interface_off();
      // alert( "socket onclose" );
  };
  image_changer = setInterval(change_image, 5000);
}


function interface_on() {
      // disable bind button
      $( "#bind_button" ).unbind();
      // enable interface to the ppsFetch process
      $( "#unbind_button" ).click( unbind_socket );
      $( "#refresh_button" ).click( socket_refresh );
      $( "#acquisition_button" ).click( start_acquisition );
      $( "input" ).prop("disabled", true);
      $( "#bind_button, #unbind_button, #refresh_button, #acquisition_button" ).toggleClass( "enabled" );
}

function interface_off() {
      // enable bind button
      $( "#bind_button" ).click( bind_socket );
      // disable interface to ppsFetch process
      $( "#unbind_button, #refresh_button, #acquisition_button" ).unbind();
      $( "input" ).prop("disabled", false);
      $( "#bind_button, #unbind_button, #refresh_button, #acquisition_button" ).toggleClass( "enabled" );
}




var images = [];
images[0] = "dialup-final.png";
images[1] = "testfig1.png";
images[2] = "PopSci-journal-history-diagram.png";
images[3] = "tot_multichannels.png";
images[4] = "intergalactic-light.png";
images[5] = "dist_edgetime.png";
images[6] = "dencity-for-web.jpg";

var x = 0;
function change_image() {
  x = (x <= 0) ? images.length - 1 : x - 1;
  d = new Date();
  // $("img").src = "intergalactic-light.png";
  $( "img" ).attr( "src", images[x] + "?" + d.getTime() );
  // alert( x + images[x]);
}

var image_changer;

// Bind stuff:

$( document ).ready( function(){
    // alert( $("#port").value );
    // $( ".help" ).css( "border", "3px solid red" );
    // alert( "sjkdhask" );

    $( ".help" ).click(function() {
      $( this ).children().toggle();
    });

    $( "#time_field" ).html( new Date() );
    setInterval( function() {$( "#time_field" ).html( new Date() );}, 1000);

    // buttons
    $( "#bind_button" ).click( bind_socket );
    $( "#bind_button" ).toggleClass( "enabled" );
    if (connection!==undefined) {
      // alert( connection );
      interface_on();
    }
} );

$( window ).unload( function() {
  if (connection!==undefined) {
    connection.close();
  }
});

