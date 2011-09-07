About:
"OpenLCB" is a library for using the OpenLCB protocol on the Arduino platform (and on AVRs more generally). Insofar as it has been tested, it has been made to work on an Arduino Uno with an MCP2515 (using hardware SPI), and on an OpenLCB Dev Board (AT90CAN128).
This library was assembled by D.E. Goodman-Wilson from the hard work of the OpenLCB team.
see: http://openlcb.sf.net

License:
See LICENSE.TXT

Features:
 * Unlimited (not really) virtual nodes permits a single piece of hardware to service several independent NodeIDs.
 * Transparent handling of alias translation.
 * Clever class organization ;)
 * Distinct layers are handled by distinct classes. The Datagram_Handler class deals with OpenLCB Datagram protocol without having to know about CAN, etc.

Documentation:
Forthcoming.

Dependencies:
"CAN" Arduino/AVR library (available from OpenLCB download site)

Using:
Copy this folder to the "libraries" folder in your Arduino sketchbook.

TODO:
This software should be considered a developers' preview, and is nothing like feature complete. Indeed, the following are all missing.
 * Datagram reception never times out; incomplete datagrams will cause a hang in datagram reception.
 * Cached aliases need to time out eventually.
 * No PCER support. (Yet.)
 * No Stream support.
 * No support for higher-level protocols (Blue/Gold, configuration, etc).
 * No built-in support for storing persistant data in EEPROM. (Is this even needed?)
 * Memory hoggish. Lots of opportunities to condense memory usage.
 * Inconsistent use of * and & paradigms for passing parameters by reference. Really? Who cares?
 * Many, many bugs to squash.

BUGS:
 * Currently, no transport-layer messages (PCER, datagrams, etc) are transmitted between virtual nodes. This is a very large problem, that requires careful attention.
