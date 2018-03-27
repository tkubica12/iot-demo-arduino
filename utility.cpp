// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "HTS221Sensor.h"
#include "LPS22HBSensor.h"
#include "LIS2MDLSensor.h"
#include "LSM6DSLSensor.h"
#include "AzureIotHub.h"
#include "Arduino.h"
#include "parson.h"
#include <assert.h>
#include "config.h"
#include "RGB_LED.h"

#define RGB_LED_BRIGHTNESS 32

DevI2C *i2c;
HTS221Sensor *ht_sensor;
LPS22HBSensor *pressure_sensor;
LIS2MDLSensor *magneto_sensor;
LSM6DSLSensor *accgyro_sensor;

static RGB_LED rgbLed;
static int interval = 2000;

static long xAvg = 0;
static long yAvg = 0;
static long zAvg = 0;
static long xMax = 0;
static long yMax = 0;
static long zMax = 0;
static long accelCount = 0;

int getInterval()
{
    return interval;
}

void setInterval(int intervalValue)
{
    interval = intervalValue;
}

void blinkLED()
{
    rgbLed.turnOff();
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    delay(100);
    rgbLed.turnOff();
}

void blinkSendConfirmation()
{
    rgbLed.turnOff();
    rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
    delay(100);
    rgbLed.turnOff();
}

void parseTwinMessage(DEVICE_TWIN_UPDATE_STATE updateState, const char *message)
{
    JSON_Value *root_value;
    root_value = json_parse_string(message);
    if (json_value_get_type(root_value) != JSONObject)
    {
        if (root_value != NULL)
        {
            json_value_free(root_value);
        }
        LogError("parse %s failed", message);
        return;
    }
    JSON_Object *root_object = json_value_get_object(root_value);

    double val = 0;
    if (updateState == DEVICE_TWIN_UPDATE_COMPLETE)
    {
        JSON_Object *desired_object = json_object_get_object(root_object, "desired");
        if (desired_object != NULL)
        {
            val = json_object_get_number(desired_object, "interval");
        }
    }
    else
    {
        val = json_object_get_number(root_object, "interval");
    }
    if (val > 500)
    {
        interval = (int)val;
        LogInfo(">>>Device twin updated: set interval to %d", interval);
    }
    json_value_free(root_value);
}

void SensorInit()
{
    i2c = new DevI2C(D14, D15);
    ht_sensor = new HTS221Sensor(*i2c);
    ht_sensor->init(NULL);
    pressure_sensor = new LPS22HBSensor(*i2c);
    pressure_sensor->init(NULL);
    magneto_sensor = new LIS2MDLSensor(*i2c);
    magneto_sensor->init(NULL);
    accgyro_sensor = new LSM6DSLSensor(*i2c, D4, D5);
    accgyro_sensor->init(NULL);
    accgyro_sensor->enableAccelerator();
    accgyro_sensor->enableGyroscope();
}

float readTemperature()
{
    ht_sensor->reset();

    float temperature = 0;
    ht_sensor->getTemperature(&temperature);

    return temperature;
}

float readHumidity()
{
    ht_sensor->reset();

    float humidity = 0;
    ht_sensor->getHumidity(&humidity);

    return humidity;
}

float readPressure()
{
    float pressure = 0;
    pressure_sensor->getPressure(&pressure);

    return pressure;
}

void measureAccel() {
    int mAcce[3];
    accgyro_sensor->getXAxes(mAcce);
    accelCount = accelCount + 1;
    xAvg = xAvg + mAcce[0];
    yAvg = yAvg + mAcce[1];
    zAvg = zAvg + mAcce[2];
    if(abs(mAcce[0]) > abs(xMax)) {
        xMax = mAcce[0];
    }
    if(abs(mAcce[1]) > abs(yMax)) {
        yMax = mAcce[1];
    }
    if(abs(mAcce[2]) > abs(zMax)) {
        zMax = mAcce[2];
    }
}

bool readMessage(int messageId, char *payload)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;

    json_object_set_string(root_object, "deviceId", DEVICE_ID);
    json_object_set_number(root_object, "messageId", messageId);

    float temperature = readTemperature();
    float humidity = readHumidity();
    float pressure = readPressure();
    int magneto[3];
    int acce[3];
    int gyro[3];
    magneto_sensor->getMAxes(magneto);
    accgyro_sensor->getXAxes(acce);
    accgyro_sensor->getGAxes(gyro);
    bool temperatureAlert = false;
    // if(temperature != temperature)
    // {
    //     json_object_set_null(root_object, "temperature");
    // }
    // else
    // {
    //     json_object_set_number(root_object, "temperature", temperature);
    //     if(temperature > TEMPERATURE_ALERT)
    //     {
    //         temperatureAlert = true;
    //     }
    // }

    // if(humidity != humidity)
    // {
    //     json_object_set_null(root_object, "humidity");
    // }
    // else
    // {
    //     json_object_set_number(root_object, "humidity", humidity);
    // }

    // if(pressure != pressure)
    // {
    //     json_object_set_null(root_object, "pressure");
    // }
    // else
    // {
    //     json_object_set_number(root_object, "pressure", pressure);
    // }

    // if(magneto != magneto)
    // {
    //     json_object_set_null(root_object, "magneto");
    // }
    // else
    // {
    //    json_object_dotset_number(root_object, "magneto.x", magneto[0]);
    //    json_object_dotset_number(root_object, "magneto.y", magneto[1]);
    //    json_object_dotset_number(root_object, "magneto.z", magneto[2]);
    // }
    
    if(acce != acce)
    {
        json_object_set_null(root_object, "acce");
    }
    else
    {
       json_object_dotset_number(root_object, "acce.now.x", acce[0]);
       json_object_dotset_number(root_object, "acce.now.y", acce[1]);
       json_object_dotset_number(root_object, "acce.now.z", acce[2]);
       json_object_dotset_number(root_object, "acce.avg.x", xAvg/accelCount);
       json_object_dotset_number(root_object, "acce.avg.y", yAvg/accelCount);
       json_object_dotset_number(root_object, "acce.avg.z", zAvg/accelCount);
       json_object_dotset_number(root_object, "acce.max.x", xMax);
       json_object_dotset_number(root_object, "acce.max.y", yMax);
       json_object_dotset_number(root_object, "acce.max.z", zMax);
       accelCount = 0;
       xAvg = 0;
       yAvg = 0;
       zAvg = 0;
       xMax = 0;
       yMax = 0;
       zMax = 0;
    }
    
    if(gyro != gyro)
    {
        json_object_set_null(root_object, "gyro");
    }
    else
    {
       json_object_dotset_number(root_object, "gyro.x", gyro[0]);
       json_object_dotset_number(root_object, "gyro.y", gyro[1]);
       json_object_dotset_number(root_object, "gyro.z", gyro[2]);
    }

    serialized_string = json_serialize_to_string_pretty(root_value);
    Serial.print(serialized_string);

    snprintf(payload, MESSAGE_MAX_LEN, "%s", serialized_string);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    return temperatureAlert;
}