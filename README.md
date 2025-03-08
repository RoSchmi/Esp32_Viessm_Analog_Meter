 ### Esp32_Viessm_Analog_Meter

This is an App for an Esp32 Dev Board which retrieves sensor data via the Viessmann Api from the Viessmann Cloud and stores the data in Azure Storage Tables in a special way that they can be graphically visualized as time series graphs with my iPhone App **'Charts4Azure'**.

![gallery](https://github.com/RoSchmi/Esp32_Viessm_DatBase_Bridge/blob/master/pictures/Heating_Graph_2.PNG)

This App is based on the code of my App https://github.com/RoSchmi/Esp32_WiFiManager_HeatingSurvey
This other App for Esp32 monitors the activity of the burner of an oil-heating
(or any other noisy thing) by measuring the sound of e.g. the heating burner. So the functions of this App are available in the App 'Esp32_Viessm_DatBase_Bridge' too.
For details visit the Github Repo of this former App. 

### The Story:
Recently we relocated to another city into a house with a Viessmann Vitodens 333F heating. 
For this heating it was easily possible to mount a VITOCONNECT 100 OPTO1
module which permanently sends sensor data of the heating via WiFi and internet to the Viessmann Cloud and makes it possible to control and monitor the heating via the 'Vicare' App from Viessmann.
Using this App one can see actual sensor data and change settings, but it is not possible to maintain historic sensor data and to display sensor data graphically as a time series graphs.
Since Viessmann provides access to the actual heating sensor data via an API I decided to make an Esp32 application to download the sensor data via the API around every minute (1440 downloads per day are free) and store the values of several sensors on Azure Storage Tables. This requires a Microsoft Azure Account, is not completely free, but very cheap (only several cents per month).
So I made this App which loads the sensor data from the Viessmann Cloud and stores the data in Azure Storage Tables in a certain way.
To display these data graphically on an Apple iPhone I use the iPhone App 'Charts4Azure' (Android version and a Microsoft Store App with minor functionality exist as well). The App 'Charts4Azure' always displays a set of 4 analog time series graphs (e.g. 4 temperature curves) and 4 On/Off time line graphs (e.g. for burner or pump activity) on one page.

This App for the Esp32 Board can be loaded on an Esp32 Board without need to change the code to adapt the App to indvidual connection parameter like WiFi-Credentials, Azure-Credentials and Viessmann Credentials.
All credentials can be entered on (e.g.) an iPhone via a Captive Portal page which is opened by the Esp32-App for one minute after powering up the Esp32 device.

After having entered the credentials one time they stay permanently on the Esp32, need not to be entered every time. Changed credentials can be entered via the Captive Portal page without need to change the credential in the program code.

This App uses an adaption of the 'Async_ConfigOnStartup.ino' as WiFi Mangager
from: -https://github.com/khoih-prog/ESPAsync_WiFiManager 
More infos about the functions and Licenses see:

https://github.com/khoih-prog/ESPAsync_WiFiManager/blob/master/examples/Async_ConfigOnDoubleReset_Multi

-https://github.com/khoih-prog/ESPAsync_WiFiManager/tree/master/examples/Async_ConfigOnStartup
 
The WiFiManager will open a configuration portal in the WLAN Access-Point for 60 seconds when powered up if the boards has stored WiFi Credentials, otherwise, the Access Point will stay indefinitely in ConfigPortal until the Router WiFi Credentials and other Credentials are entered.
When the Esp32 starts up it provides an Access Point after about one minute to which you must connect with your iPhone. 
The SSID of this Access point is like e.g. ESP32_xxxxxx, if a password is requested (only the first time), the passwort is "My" + the SSID-Name, e.g. MyESP32_xxxxxx.

Although the necessary custom configuration (e.g. Credentials) can be made through the WiFi Manager, some settings remain that can be configured via the configuration files config.h and config_secret.h:
The adapted config_secret.h for your needs/system must be provided by copying and renaming of config_secret_template.h to config_secret.h. In config_secret.h you can preset your first time custom Credentials for your personal WLAN-Network, Azure Storage Account, Viessmann Client-Id and Viessmann Refresh Token (Can be entered through WiFi-Manager as well). 
In config.h the timezone offset and the parameter for dayligthsavingtime must be changed if you are not located in Germany.

To access the Viessmann Api through this Esp32 App you need a 'Client-Id' and a 'Refresh Token' (valid for 180 days), which have to be entered in config_secret.h or through the WiFi Manager Captive Portal Page. 
The Refresh token enables the App to repeatedly load new access tokens which are needed to download data through the Viessmann Api. Access Tokens are valid only for one hour.

The Client-Id, Access-Token (valid for 1 hour) and Refresh-Token can be retrieved from: 

https://developer.viessmann.com/start/pricing.html (Basic -> Get started)

To get a Refresh-Token you can use my C# Windows Application:

https://github.com/RoSchmi/RoSchmiViessmannApiTest together with an Internet Browser 

More valuable information can be found here:

https://www.rustimation.eu/index.php/a-viessmann-api-und-node-red/

https://github.com/philippoo66/optolink-splitter

https://github.com/openv

https://github.com/philipdaubmeier/GraphIoT

To learn how to get an Azure Account, a Storage Account and a Storage Account key have a look here: 

https://azureiotcharts.home.blog/getting-started-guide/
