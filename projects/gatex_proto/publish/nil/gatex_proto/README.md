 -  identity
     -  contains information of the nodes and vertex types.
     -  to be used for checking compatibility
     -  only contains type indices
     -  only used by non-gui
 -  graph
     -  contains active graph
     -  contains ids and aliases only
     -  used by gui and non-gui
     -  alias is only going to be used for feedback/delay node
     -  when received by gui, it should clear and replace the current graph
     -  TODO: maybe attach alias to each vertex?
 -  gui_metadata
     -  only used for gui
     -  contains panel detail
         -  pin label/color/icon
         -  node label/pin/control information
     -  when received by gui, it should clear the current graph
 -  renderer_metadata
     -  contains metadata for renderer
     -  for imgui, contains position of each node
 -  node_update
     -  only used for gui
     -  responsible in highlighting active/running nodes
 -  control_update
     -  gui <-> server
     -  mainly gui -> server to send the new values of the control edges
 -  Dump
     -  not intended to be transmitted
     -  to be used for storing to file
     -  should contain the following:
         -  identity
         -  graph
         -  gui_metadata
         -  renderer_metadata (bytes)
 -  Serialize Message Type
     -  when gui wants to save something, it should simply request it to be serialized by the server.
     -  gui should not deserialize anything.
     -  request should contain only renderer_metadata. all other information should come from the server
 -  Load Message Type
     -  gui should read the file as it is and send the buffer to the server
     -  server should parse the buffer and send the following:
         -  gui_metadata
         -  renderer_metadata
     -  TODO: should i just send Dump?
 -  On Connect Message Type 
     -  server should send the following:
         -  gui_metadata
         -  graph
         -  renderer_metadata
     -  TODO: should i just send Dump?