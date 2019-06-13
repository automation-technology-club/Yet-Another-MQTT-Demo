# Over The Air Firmware Updates

Setting up SPIFFS on the D1 Mini is easy to do by following the step by step instructions found here:  
 Information on setting up SPIFFS can be found here:  
    [Video](https://www.youtube.com/watch?v=jIOTzaeh7fs)  
    [ESP8266FS Plugin](https://github.com/esp8266/arduino-esp8266fs-plugin/releases)  

You do need to select a Flash Size in the Arduino IDE tools menu, this is important - because if you don't you could over write the flash area, and have to start again.  
This could be a problem if you are updating a device in a remote location, or one that is hard to get to.  

The frist time you program your device, you'll also need to install the config.json file.  
Once all that is done you can deploy, and do Over the Air updates easy from the Arduino IDE.  

If you look at the tools menu, and the port you should now see a new "network" port with the host name you set in the config.json file.  

## Config File

For this project our conf.json file is pretty simple, but can be expanded as needed.  

```
{
    "mqtt_server":"broker.hivemq.com",
    "mqttname":"LeRoy",
    "host":"LeRoyLEDMatrix"
}
```

This is a simple json formated file - you'll need three pieces of information.  
a mqtt_server (broker) url, here we are using hivemq.com, but any broker should work,  
Next we see mqttname - and for some reason I'm calling my display "LeRoy"  
Here the mqttname take on a slightly different mean for the door bell sketch. For the door bell this is the name of the display we want to talk to.  
Finally we have a "host", this is the name of the device, this is also the name that shows up in the tools/port/network ports.  
For the door bell, the host name is hard wired, time permitting we can make some changes to the sketches, and update the firmware ota.  

