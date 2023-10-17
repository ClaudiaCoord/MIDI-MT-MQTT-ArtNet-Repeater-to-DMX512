# MIDI-MT -> MQTT+ArtNet Repeater to DMX512

MIDI-MT: MQTT + ArtNet Repeater to DMX512 networks. Host lighting controls.  
See [MIDI-MT software](https://claudiacoord.github.io/MIDI-MT/)  
More details about diagrams and connection details can be found in the [MIDI-MT wiki](https://github.com/ClaudiaCoord/MIDI-EasyControl-to-Mackie-translator-for-Premiere-Pro/wiki/EN-Repeater-DMX512-ARTNET-MQTT)  

To implement the integration of ArtNet, MQTT and DMX network technologies into a single lighting control system, you can use not only equipment from well-known manufacturers. It is much cheaper to use similar Arduino boards with support for WiFi technology, designed on ESP chipsets. For example ESP8266, ESP32, etc. for these purposes.  

At the same time, it turns out to get a device with maximum capabilities at a minimum price. The budget of such a device will cost about the cost of one track lamp, in other words, its cost will be approximately 10-15% of the price of similar branded equipment.  

One of these designs, which serves to expand the coverage of a network of smart devices, runs under the control of the MIDI-MT program, for which it was specially developed. Simplicity of execution and the wide availability of electronic components make it universal. This is a repeater that collects control information about the switching status of lighting devices from the ArtNet and MQTT networks, then the information is summarized and transmitted via the DMX protocol to the wired network, through which the actual lighting control occurs. Use requires the presence of a wired DMX512 network and a connection point to it.

![MIDI-MT MQTT+ArtNet Repeater to DMX512](https://raw.githubusercontent.com/ClaudiaCoord/MIDI-MT-MQTT-ArtNet-Repeater-to-DMX512/main/docs/Images/DMX-Artnet-Wifi-Repeater_scheme.png)  

Edit and rename `src/config.h.default`  to `src/config.h`.  

__mosquitto base configuration:__  

```
per_listener_settings true

listener 1883
protocol mqtt
socket_domain ipv4

user mosquitto

allow_anonymous false
retain_available true
use_username_as_clientid true
allow_zero_length_clientid true
auto_id_prefix auto-
queue_qos0_messages false
sys_interval 90
autosave_interval 1800
autosave_on_changes true

persistence true
persistent_client_expiration 1m
persistence_location /var/lib/mosquitto/

password_file /etc/mosquitto/pass.cfg
acl_file /etc/mosquitto/acl.cfg
pid_file /run/mosquitto/mosquitto.pid

log_dest syslog
log_type warning

listener 9001
protocol websockets
http_dir /srv/git/linuxconfig.git/mqttweb

```

__mosquitto acl configuration:__  

```
topic read $SYS/#
pattern readwrite sensor/%u/#
pattern write $SYS/broker/connection/%c/state

# !!!this!!! - add Repeater (art-dmx-XXX-XXX) to read access!
user art-dmx-0-11
topic read #

```
