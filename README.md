# MIDI-MT-MQTT-ArtNet-Repeater-to-DMX512

MIDI-MT: MQTT + ArtNet Repeater to DMX512 networks. Host lighting controls.  
see [MIDI-MT software](https://claudiacoord.github.io/MIDI-MT/)  

To implement the integration of ArtNet, MQTT and DMX network technologies into a single lighting control system, you can use not only equipment from well-known manufacturers. It is much cheaper to use similar Arduino boards with support for WiFi technology, designed on ESP chipsets. For example ESP8266, ESP32, etc. for these purposes.  

At the same time, it turns out to get a device with maximum capabilities at a minimum price. The budget of such a device will cost about the cost of one track lamp, in other words, its cost will be approximately 10-15% of the price of similar branded equipment.  

One of these designs, which serves to expand the coverage of a network of smart devices, runs under the control of the MIDI-MT program, for which it was specially developed. Simplicity of execution and the wide availability of electronic components make it universal. This is a repeater that collects control information about the switching status of lighting devices from the ArtNet and MQTT networks, then the information is summarized and transmitted via the DMX protocol to the wired network, through which the actual lighting control occurs. Use requires the presence of a wired DMX512 network and a connection point to it.
