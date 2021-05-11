# RS485 WiFi EPEver

This is a DIY RS485 to wifi adaptor which allows you to connect to your EPEver controller over wifi.

<img src="images\Board-Image.PNG" alt="https://github.com/chickey/RS485-WiFi-EPEver/blob/master/images/Board-Image.PNG" style="zoom: 25%;" />

Many EPEver solar charge controllers have an RS485 communication port on them so you can connect a PC or mobile device and see it's live data as well as statistical data.  The code also allows this data to be pushed to an MQTT broker or an Influx DB which could then be visualised via a grafana dashboard.

I am currently tweaking and modifying the code so it shall evolve with more features as time goes on.

I took great inspiration from the following project:-

https://github.com/glitterkitty/EpEverSolarMonitor 

I am also using the ESPUI project which without this I wouldn't have had the doody interfaces :-)

# Tested Hardware

- LS1024B

- Tracer 3210A

- MPPT Tracer 1210 

- Tracer AN Series

  - 2210AN, 3210AN and 4210AN 

- TRITON Series with RS485 module



# Updating

Ensure when flashing the Wemos board that either it is not plugged into the RS485 adaptor or that that the board is not connected to the charge controller to avoid damage.

You can now update remotely by browsing to http://IP/ota and browsing to the precompiled binary file and then selecting upload.



# Discussion

I'm always up for suggestions either via github or if you wish to chat with like minded people and pick people's brains on their setups i have setup a discord server

https://discord.gg/kBDmrzE