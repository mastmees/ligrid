# ligrid
This is a copy of my sourceforge repository, the aim of this project
was to produce better analog front-end, and linux support for Frank
Kooiman's lightning radar network. Frank suddenly passed away, and
the project faded.

Enjoy the trip down the memory lane.

---

LiGrid is a lightning strike data collection system. It started out as
data gathering front-end for Frank Kooiman's Lightning Radar network
(see http://members.home.nl/fkooiman/lightning/) which is written in Visual
Basic. At a time of writing Frank is working on his software to support
LiGrid nodes for collecting data. LiGrid currently also simulates Frank's
Lightning Radar node on the network so that other nodes can use it for
triangulation data source.

Other lightning detection networks exists, but Frank Kooiman's is somewhat
unique. First of all, the hardware is very cheap - basically an audio
frequency amplifier built on an op-amp, connected to line input of audio
interface on PC. Compare this to commercially available hardware that easily
exceeds 1000 USD mark for a ground station. See the documentation directory
for hardware description and schematics.

In Frank's system the signal is taken from two loop antennas which are
mounted at 90 degree angle to each other. The signal difference between
the loops is used to calculate a bearing of a signal, and two or more
stations are then used to cross-check each other and triangulate to
position a lightning strike. This method does not need highly precise
timing, avoiding the need for GPS receiver that is used in some other
detector networks. Of course, adding a GPS receiver support and
additional network protocols would allow the same node to participate
in multiple different detector networks.

To run LiGrid, you need a working OSS setup on your machine, antenna and
amplifier. Makefile is provided that by default builds ligrid executable.
Additional targets are clean, bare, restart, and kill (stops a running
executable).

issuing make in utils directory will build two small utilities that
may help you to diagnose hardware problems in early stage of development.
