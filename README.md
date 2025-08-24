# B-Works
Prototipe e-Ink Bus Arrival Display (OLED 0.9inch)

This project is a real-time bus arrival time display built using a Wemos D1 Mini microcontroller. It is designed to be a power-efficient, easy-to-configure, and reliable device for displaying bus arrival information from the LTA DataMall API.

**Main Components**
Wemos D1 Mini: An ESP8266-based microcontroller that acts as the "brain" of the device. Wemos was chosen for its compact size, built-in Wi-Fi capabilities, and Deep Sleep feature to conserve power.

0.96-inch I2C OLED Display: A small display with a 128x64 pixel resolution that is highly efficient at displaying simple text clearly.

Push Button: An optional but recommended component to allow Wi-Fi configuration resets at any time.

LTA DataMall API: The primary data source providing real-time bus arrival information.


**System Workflow**
This project operates in an efficient cycle:

Initial Configuration (Setup): When first powered on, the Wemos will act as a Wi-Fi access point (access point) named "BusDisplayAP." You can connect to this network from your phone or laptop, and a web portal will automatically appear.

Credential Input: Through this web portal, you can enter your Wi-Fi credentials (SSID and password) and API information such as the LTA Account Key and Bus Stop Code. WiFiManager will permanently store this information in the Wemos' memory.

Data Retrieval: After connecting to Wi-Fi, the Wemos will send an HTTP GET request to the Bus Arrival API endpoint in LTA DataMall, using the Account Key and Bus Stop Code you entered.

JSON Parsing: The device will receive a response in JSON format and use the ArduinoJson library to extract relevant information, such as the bus number and estimated arrival time.

Display: The parsed data will be displayed on the OLED screen in a simple, easy-to-read format (e.g., "Bus 14:5 min").

Power Saving Mode: After updating the screen, Wemos will enter Deep Sleep mode for 60 seconds. In this mode, power consumption is minimal, making it ideal for battery-powered projects.

Repeat Cycle: After 60 seconds, Wemos will automatically wake up, repeating the data capture and screen update process, ensuring information is always up-to-date.
