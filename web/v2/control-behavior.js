var port = 1987;
var listener_id;
var connection = undefined;
var upper_fields;
var bind_button, unbind_button, refresh_button, start_acquisition_button, time_field;
var socket_id, console_log;
var built_clients;
var acquisition_started;


function parse_handshake_message(event) {
  var d = event.data; //.slice(0,-1); // CHECK: is slice needed?
  m = d.split(":");
  switch (m[0]) {
    case "SET_CLIENT_ID":
          listener_id = parseInt(m[1]);
          $( "#socket_id" ).val( listener_id );
          socket_refresh();
          setup_connection(connection);
          interface_on();
          image_changer = setInterval(change_image, 5000); // TODO: kick when DQM is ready
          break;
    default:
          append_to_console( '<p>wierd message @ handshake<\p>' );
  }
}


function parse_message(event) {
  var d = event.data; // .slice(0,-1);
  m = d.split(":");
  switch (m[0]) {
    case "SET_CLIENT_ID":
          listener_id = parseInt(m[1]);
          $( "#socket_id" ).val( listener_id );
          socket_refresh();
          break;
    case "CLIENTS_LIST":
          var listeners = m[1].split(';');
          // TODO: do something
          break;
    case "DQM_READY":
          // the filename of new DQM plot is in m[1]
          d = new Date();
          $( "#output1" ).attr( "src", m[1] + "?" + d.getTime() );
          $( "#output1" ).panzoom("reset");
          break;
    case "THIS_CLIENT_DELETED":
          unbind_socket();
          break;
    case "OTHER_CLIENT_DELETED":
          if (m[1]==listener_id) {
            unbind_socket();
          }
          else append_to_console("<p>Socket " + m[1]+" successfully deleted!</p>");
          break;
    case "PING_ANSWER":
          alert(m[1]);
          break;
    case "ACQUISITION_STARTED":
          append_to_console( "<p>Acquisition process successfully launched!</p>" );
          // alert("Acquisition process successfully launched!");
          $( "#start_acquisition_button" ).removeClass().unbind();
          $( "#stop_acquisition_button" ).attr( "class", "enabled" ).click( stop_acquisition ) ;
          acquisition_started = true;
          break;
    case "ACQUISITION_STOPPED":
          append_to_console( "<p>Acquisition process terminated!</p>" );
          // alert("Acquisition process terminated!");
          $( "#stop_acquisition_button" ).removeClass().unbind();
          $( "#start_acquisition_button" ).attr( "class", "enabled" ).click( start_acquisition ) ;
          acquisition_started = false;
          break;
    case "EXCEPTION":
          append_to_console( "<p>" + d + "</p>" );
          break;
    case "MASTER_DISCONNECT":
          // alert("ALERT:\nMaster disconnected!");
          append_to_console( "<p>ALERT:\nMaster disconnected!</p>" );
          // restore_init_state();
          break;
    default:
          append_to_console( "<p>Received unknown message:" + d + "</p>" );
          break;
  }
}




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

  connection.close();
  connection = undefined;

  append_to_console( '<p>UNBOUNDED!<\p>' );
}


function socket_refresh() {
  if (connection===0 || connection===undefined) return;
  if (listener_id<0) return;
  
  send_message(connection, "WEB_GET_CLIENTS:" + listener_id);
  bind_time.style.color = "black";
  bind_time.innerHTML = new Date();
}


function start_acquisition() {
  send_message(connection, "START_ACQUISITION:" + listener_id );
}

function stop_acquisition() {
  send_message(connection, "STOP_ACQUISITION:" + listener_id );
}



function setup_connection(connection) {
  connection.onopen = function () {
    // what a hell
    append_to_console( '<p class="incoming">GOT: Connection opened<\p>' );
    // interface_on();
    // image_changer = setInterval(change_image, 5000);
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
      alert("connection onerror"); // Parse error, write error message to console
      // connection.close();
      connection = undefined;
      alert( connection );
      connection = undefined;
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
      window.clearInterval( image_changer );
      // alert( "socket onclose" );
  };

  append_to_console( '<p>CONNECTION HANDLERS ARE SET UP<\p>' );
  // alert("CONNECTION HANDLERS ARE SET UP");
}





function bind_socket() {
  connection_uri = $("#connection_uri").val();
  // hostname = $("#hostname").val();

  // alert( connection );
  // alert( hostname + ":" + port );
  // if (connection==0) { alert("no connection") };
  append_to_console( '<p>Start Bind<\p>' );

  if (typeof connection !== "undefined") {
    alert("repeated connection 2");
    console.error("Wrong (or multiple) attempt made to bind! Aborting.");
    // connection = 0;
    return;
  }

  append_to_console( '<p>Connection is undefined<\p>' );

  // hostname = 'localhost';
  //hostname = '137.138.105.83';
  try {
    connection = new WebSocket(connection_uri);
  }
  catch (e) {
    alert(e);
  }

  append_to_console( '<p>New connection created:' + connection + '<\p>' );

  // prepare handshake response
  connection.onmessage = function(event) {
    append_to_console( '<p class="incoming">GOT @ handshake: ' + event.data + "<\p>" );
    //SET_CLIENT_ID:ID
    parse_handshake_message(event); // if it receives SET_CLIENT_ID:ID the connection is set up for run
  };
  connection.onopen = function(event) {
    append_to_console( '<p>Connection opened @ handshake<\p>' );
    send_message( connection, "ADD_CLIENT:1" ); // Handshake call
    append_to_console( '<p>Sent the handshake<\p>' );
  }
  connection.onerror = function(event) {
    append_to_console( '<p>Connection error @ handshake<\p>' );
    // TODO: print the error message
  }
  connection.onclose = function(event) {
    append_to_console( '<p>Connection closed @ handshake<\p>' );
  }
}


function interface_on() {
      // disable bind button
      $( "#bind_button" ).unbind().removeClass();
      // enable interface to the ppsFetch process
      $( "#unbind_button" ).click( unbind_socket );
      $( "#refresh_button" ).click( socket_refresh );
      $( "#start_acquisition_button" ).click( start_acquisition );
      $( "#stop_acquisition_button" ).unbind();
      $( "input#connection_uri" ).prop("disabled", true);
      $( "#unbind_button, #refresh_button, #start_acquisition_button" ).attr( "class", "enabled" );//toggleClass( "enabled" );
      $( "#stop_acquisition_button" ).removeClass();
}

function interface_off() {
      // enable bind button
      $( "#bind_button" ).click( bind_socket ).attr( "class", "enabled" );
      // disable interface to ppsFetch process
      // $("#start_acquisition_button").html( "Start acquisition" ); // reset acquisition button
      $( "#unbind_button, #refresh_button, #start_acquisition_button, #stop_acquisition_button" ).unbind();
      $( "input#connection_uri" ).prop("disabled", false);
      $( "#unbind_button, #refresh_button, #start_acquisition_button, #stop_acquisition_button" ).removeClass();
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
  $( "#output1" ).attr( "src", images[x] + "?" + d.getTime() );
  $( "#output1" ).panzoom("reset");
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

    $( "#output1" ).panzoom({$zoomIn: $("#zoom-in"), $zoomOut: $("#zoom-out")});

    // buttons
    $( "#bind_button" ).click( bind_socket );
    $( "#bind_button" ).toggleClass( "enabled" );
    if (typeof connection !== "undefined") {
      // alert( connection );
      interface_on();
    }

} );

$( window ).unload( function() {
  if (typeof connection !== "undefined") {
    connection.close();
  }
});

