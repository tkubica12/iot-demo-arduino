// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "DevKitMQTTClient.h"

#include "config.h"
#include "utility.h"
#include "SystemTickCounter.h"

static bool hasWifi = false;
int messageCount = 1;
static bool messageSending = true;
static uint64_t send_interval_ms;
static uint64_t messageTimer = 0;
static int screenNumber = 0;
static char receivedMessage[32];


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void InitWifi()
{
  Screen.print(2, "Pripojuji...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Posilame... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "Nemam Wi-Fi\r\n ");
  }
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    blinkSendConfirmation();
  }
}

static void MessageCallback(const char* payLoad, int size)
{
  blinkLED();
  strcpy(receivedMessage, payLoad);
  messageTimer = SystemTickCounterRead();
}

static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
  char *temp = (char *)malloc(size + 1);
  if (temp == NULL)
  {
    return;
  }
  memcpy(temp, payLoad, size);
  temp[size] = '\0';
  parseTwinMessage(updateState, temp);
  free(temp);
}

static int  DeviceMethodCallback(const char *methodName, const unsigned char *payload, int size, unsigned char **response, int *response_size)
{
  Serial.printf("Message method: %s", methodName);
  LogInfo("Try to invoke method %s", methodName);
  const char *responseMessage = "\"Successfully invoke device method\"";
  int result = 200;

  if (strcmp(methodName, "start") == 0)
  {
    LogInfo("Start sending temperature and humidity data");
    messageSending = true;
  }
  else if (strcmp(methodName, "stop") == 0)
  {
    LogInfo("Stop sending temperature and humidity data");
    messageSending = false;
  }
  else
  {
    LogInfo("No method %s found", methodName);
    responseMessage = "\"No method found\"";
    result = 404;
  }

  *response_size = strlen(responseMessage);
  *response = (unsigned char *)malloc(*response_size);
  strncpy((char *)(*response), responseMessage, *response_size);

  return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  Screen.init();
  Screen.print(0, "Azure je super");
  Screen.print(2, "Startuji...");
  
  Screen.print(3, " > Serial");
  Serial.begin(115200);

  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWifi();
  if (!hasWifi)
  {
    return;
  }

  LogTrace("HappyPathSetup", NULL);

  Screen.print(3, " > Sensors");
  SensorInit();

  Screen.print(3, " > IoT Hub");
  DevKitMQTTClient_Init(true);

  DevKitMQTTClient_SetOption(OPTION_MINI_SOLUTION_NAME, "GetStarted");
  DevKitMQTTClient_SetSendConfirmationCallback(SendConfirmationCallback);
  DevKitMQTTClient_SetMessageCallback(MessageCallback);
  DevKitMQTTClient_SetDeviceTwinCallback(DeviceTwinCallback);
  DevKitMQTTClient_SetDeviceMethodCallback(DeviceMethodCallback);

  send_interval_ms = SystemTickCounterRead();
}

bool IsButtonClicked(unsigned char ulPin)
{
    pinMode(ulPin, INPUT);
    int buttonState = digitalRead(ulPin);
    if(buttonState == LOW)
    {
        return true;
    }
    return false;
}

void loop()
{
  if (hasWifi)
  {
    if (messageSending && 
        (int)(SystemTickCounterRead() - send_interval_ms) >= getInterval())
    {
      // Send teperature data
      char messagePayload[MESSAGE_MAX_LEN];

      bool temperatureAlert = readMessage(messageCount++, messagePayload);
      EVENT_INSTANCE* message = DevKitMQTTClient_Event_Generate(messagePayload, MESSAGE);
      DevKitMQTTClient_Event_AddProp(message, "temperatureAlert", temperatureAlert ? "true" : "false");
      DevKitMQTTClient_SendEventInstance(message);
      
      send_interval_ms = SystemTickCounterRead();
    }
    else
    {
      DevKitMQTTClient_Check();
    }
  }

  if (messageSending && 
        (int)(SystemTickCounterRead() - send_interval_ms) >= 100)
  {
    measureAccel();
  }

  if(IsButtonClicked(USER_BUTTON_B) && screenNumber == 0)
  {
      if(messageSending) {
        messageSending = false;
        Screen.print(2, "Zastaveno... \r\n");
      }
      else {
        messageSending = true;
        Screen.print(2, "Posilame... \r\n");
      }
      delay(250);
  }

  if(IsButtonClicked(USER_BUTTON_B) && screenNumber == 1)
  {
      int intervalValue = getInterval();
      switch(intervalValue)
      {
        case 200:
          setInterval(2000);
          Screen.print(1, "  200 ms \r\n");
          Screen.print(2, "> 2 s \r\n");
          Screen.print(3, "  1 m \r\n");
          break;
        case 2000:
          setInterval(60000);
          Screen.print(1, "  200 ms \r\n");
          Screen.print(2, "  2 s \r\n");
          Screen.print(3, "> 1 m \r\n");
          break;
        case 60000:
          setInterval(3600000);
          Screen.print(1, "  2 s \r\n");
          Screen.print(2, "  1 m \r\n");
          Screen.print(3, "> 1 h \r\n");
          break;
        case 3600000:
          setInterval(200);
          Screen.print(1, "> 200 ms \r\n");
          Screen.print(2, "  2 s \r\n");
          Screen.print(3, "  1 m \r\n");
          break;
      }
      delay(250);
  }
  
  if(IsButtonClicked(USER_BUTTON_A) && screenNumber == 0)
  {
      screenNumber = 1;
      Screen.print(0, "Interval \r\n");
      switch(getInterval())
      {
        case 200:
          Screen.print(1, "> 200 ms \r\n");
          Screen.print(2, "  2 s \r\n");
          Screen.print(3, "  1 m \r\n");
          break;
        case 2000:
          Screen.print(1, "  200 ms \r\n");
          Screen.print(2, "> 2 s \r\n");
          Screen.print(3, "  1 m \r\n");
          break;
        case 60000:
          Screen.print(1, "  200 ms \r\n");
          Screen.print(2, "  2 s \r\n");
          Screen.print(3, "> 1 m \r\n");
          break;
        case 3600000:
          Screen.print(1, "  2 s \r\n");
          Screen.print(2, "  1 m \r\n");
          Screen.print(3, "> 1 h \r\n");
          break;
      }
      delay(250);
  }

  if(IsButtonClicked(USER_BUTTON_A) && screenNumber == 1)
  {
      screenNumber = 2;
      Screen.print(0, "Prijem zprav \r\n");
      Screen.print(1, " \r\n");
      Screen.print(2, " \r\n");
      Screen.print(3, " \r\n");
      delay(250);
  }

  if(IsButtonClicked(USER_BUTTON_A) && screenNumber == 2)
  {
      screenNumber = 0;
      Screen.print(0, "Azure je super \r\n");
      InitWifi();
      if(messageSending) {
        Screen.print(2, "Posilame... \r\n");
      }
      else {
        Screen.print(2, "Zastaveno... \r\n");
      }
      Screen.print(3, "> IoT Hub \r\n");
      delay(250);
  }

  if(((int)(SystemTickCounterRead() - messageTimer) <= 5000) && screenNumber == 2) {
    char messageBuff[32];
    snprintf(messageBuff, 32, "%s\r\n", receivedMessage);
    Screen.print(2, messageBuff);
  }
  if(((int)(SystemTickCounterRead() - messageTimer) > 5000) && screenNumber == 2) {
    Screen.print(2, "  \r\n");
  }

  delay(10);
}
