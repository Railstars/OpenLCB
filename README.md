About
-----
"OpenLCB" is a library for using the OpenLCB protocol on the Arduino platform (and on AVRs more generally). Insofar as it has been tested, it has been made to work on an Arduino Uno with an MCP2515 (using hardware SPI), and on an OpenLCB Dev Board (AT90CAN128).
This library was assembled by D.E. Goodman-Wilson from the hard work of the OpenLCB team.
see: http://openlcb.sf.net

License
-------
See LICENSE.TXT

Features
--------
 * Unlimited (not really) virtual nodes permits a single piece of hardware to service several independent NodeIDs.
 * Transparent handling of alias translation.
 * Clever class organization ;)
 * Distinct layers are handled by distinct classes. The Datagram_Handler class deals with OpenLCB Datagram protocol without having to know about CAN, etc.

Documentation
-------------
Forthcoming.

Dependencies
------------
"CAN" Arduino/AVR library (available from OpenLCB download site)

Using
-----
Copy this folder to the "libraries" folder in your Arduino sketchbook.

TODO
----
This software should be considered a developers' preview, and is nothing like feature complete. Indeed, the following are all missing.
 * Need to seperate the VirtualNode abstract base class from the X_Handler classes, which should be structured as mixins. A virtual node will inhert VirtualNode, and whichever Handlers it needs.
 * Virtual Nodes are not informed whether they are in the inhibited or permitted state. I've added a mechanism in NodeID to do this: When NodeID is initialized, the owning node may move to the permitted state; when it is not, the owning node must move to the inhibited state. This behavior is not yet fully implemented
 * An initialzied (alias allocated) NodeID that has had it's NID changed needs to emit an AMR and then an AMD
 * Two identical NodeIDs that allocate at exactly the same time will not catch this condition, and will allocate the same alias. Grr! Not sure how to handle this case, because a node cannot differentiate between a self-generated CID/RID and an identical CID/RID generated by a different node!
 * Cached aliases need to time out eventually.
 * No Stream support.
 * No support for higher-level protocols (Blue/Gold, configuration, etc).
 * No built-in support for storing persistant data in EEPROM. (Is this even needed?)
 * Memory hoggish. Lots of opportunities to condense memory usage.
 * Inconsistent use of * and & paradigms for passing parameters by reference. Really? Who cares?
 * Many, many bugs to squash.

Discussion
----------

You can get support for the OpenLCB software at the Railstars [Support Forum](http://support.railstars.com/index.php?p=/categories/openlcb)

[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/Railstars/openlcb/trend.png)](https://bitdeli.com/free "Bitdeli Badge")