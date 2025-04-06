# Medicine Reminder and Dispenser System

This project implements a **Medicine Reminder and Dispenser System** built with an **ESP32**, **TFT Display**, **Servos**, and the **Blynk App**. It helps users remember to take their medicine at scheduled times by providing visual and auditory alerts. Additionally, it controls servos to dispense the medicine when the reminder is triggered.

## Features:
- **Medicine Reminder Notifications**: Reminds users to take their medicine at preset times with a visual notification on the TFT screen and an audible alert via buzzer/LED.
- **Servo-Controlled Medicine Dispensing**: Dispenses medicine by controlling servos when the reminder is triggered. Users can also manually reset servos using physical switches.
- **Time-based Scheduling**: Set start times and intervals for medicine reminders, which the system checks in real time.
- **Blynk Integration**: Remote control via the Blynk app to configure reminders, receive notifications, and monitor system status.
- **Real-time Clock**: Uses NTP (Network Time Protocol) to synchronize time with IST (Indian Standard Time).

## Components:
- **ESP32**: The main microcontroller controlling the system and providing Wi-Fi connectivity for Blynk integration.
- **TFT Display**: Used to show reminders, real-time clock, and status updates to the user.
- **Servos**: Used to control the dispensing mechanism for the medicine.
- **LED/Buzzer**: Provide visual and audible alerts for medicine reminders.
- **Switches**: Used for confirming medicine consumption, resetting reminders, or manually controlling the dispensing servos.
- **Wi-Fi**: Provides connectivity for remote control via the Blynk app and NTP time synchronization.

## Libraries Used:
- **Blynk**: For remote control via the Blynk app.
- **TFT_eSPI**: For controlling the TFT display.
- **ESP32Servo**: For controlling the servos that dispense the medicine.
- **NTPClient**: For synchronizing time using NTP.

## Installation:

### Step 1: Clone the Repository
Clone this repository to your local machine:
```bash
git clone https://github.com/yourusername/medicine-reminder-device.git
```
## Step 2: Install Libraries
Install the following libraries using the Arduino Library Manager:

- **Blynk**
- **TFT_eSPI**
- **ESP32Servo**
- **NTPClient**

## Step 3: Set Up Blynk
1. Download and install the [Blynk app](https://blynk.io) on your phone.
2. Create a new project in the app and note down the **Auth Token**.
3. Set up the project with two virtual pins (V3, V4) for setting the first medicine's details (name, start time, and interval), and similarly for the second medicine using V5, V6, and V12.
4. Update the `BLYNK_AUTH_TOKEN` in the code with your generated Auth Token.

## Step 4: Wi-Fi Setup
Update the following Wi-Fi credentials in the code:

```cpp
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASS "your-wifi-password"
```

## Step 5: Upload Code to ESP32
Upload the code to your **ESP32** using the Arduino IDE or PlatformIO.

## Usage:
- **Blynk App**: Set up medicine reminders, enter names, start times, and intervals. Receive notifications via the app when it's time to take medicine.
- **TFT Display**: The display will show medicine names, their respective start times, and a countdown for the reminders.
- **Buttons**: The physical switches allow you to:
  - Confirm medicine consumption.
  - Reset servos and stop the alert.
  - Manually control the servo dispensing mechanism.

## Functions Explained:
- **`BLYNK_WRITE(V3)`, `BLYNK_WRITE(V5)`**: Functions to input the names of the medicines via the Blynk app.
- **`BLYNK_WRITE(V4)`, `BLYNK_WRITE(V6)`**: Functions to set the start time for each medicine.
- **`BLYNK_WRITE(V11)`, `BLYNK_WRITE(V12)`**: Functions to set the interval for each medicine reminder.
- **`checkReminder()`**: Continuously checks if it’s time to remind the user to take medicine, based on the current time and set intervals.
- **`servoControl()`**: Controls the servos to dispense the medicine when the reminder is triggered.
- **`resetAlert()`**: Resets the alert system and clears the display after confirmation.

## Auto Reset:
- **Daily Reset**: At midnight, the system automatically resets reminders and alerts.
- **Manual Reset**: Use the switches to reset the system, clear reminders, and stop any active alerts.

## Future Improvements:
- Integration with voice assistants (Alexa, Google Assistant) for hands-free operation.
- Support for multiple medicines with customized reminder types.
- Enhanced alert system with SMS, email, or other messaging services.

## License:
This project is open source and available under the **MIT License**.

---

## Troubleshooting:
- **Wi-Fi Not Connecting**: Ensure the Wi-Fi credentials are correct, and the ESP32 is connected to a stable network.
- **Servo Issues**: Ensure the servos are connected properly, and power is sufficient.
- **Blynk Not Working**: Verify that the Auth Token is correctly set in the code and that your device is correctly linked in the Blynk app.

## Teammate:
- **Sidharth Sajith

