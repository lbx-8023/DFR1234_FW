#include "I2CHub.h"


String I2CHub::getResStr()
{
    return data;
}
bool GestureFaceDetectionI2CHub::init()
{
    name = "GestureFaceDetectionSenso";
    gfd = new DFRobot_GestureFaceDetection_I2C(0x72);
    return gfd->begin(&Wire1);
}
void GestureFaceDetectionI2CHub::callback()
{
    static uint16_t faceX = 0, faceY = 0, faceScore = 0, gestureType = 0, gestureScore = 0;

    char tempStr[128];
    if (gfd->getFaceNumber() > 0)
    {
        faceX = gfd->getFaceLocationX();
        faceY = gfd->getFaceLocationY();
        faceScore = gfd->getFaceScore();
        if(gfd->getGestureType()){
            gestureType = gfd->getGestureType();
        }
        gestureScore = gfd->getGestureScore();
        sprintf(tempStr, "\"FaceX\":%d,\"FaceY\":%d,\"GestureType\":%d", faceX, faceY, gestureType);
        data = String(tempStr);
    }
    else
    {
        sprintf(tempStr, "\"FaceX\":%d,\"FaceY\":%d,\"GestureType\":%d", faceX, faceY, gestureType);
        data = String(tempStr);
    }
}

bool GR10_30I2CHub::init()
{
    name = "GR10_30I2CHub";
    data = "";
    gr10_30 = new DFRobot_GR10_30(/*addr = */ GR10_30_DEVICE_ADDR, /*pWire = */ &Wire1);
    // gr10_30->enGestures(GESTURE_UP | GESTURE_DOWN | GESTURE_LEFT | GESTURE_RIGHT | GESTURE_FORWARD | GESTURE_BACKWARD | GESTURE_CLOCKWISE | GESTURE_COUNTERCLOCKWISE | GESTURE_CLOCKWISE_C | GESTURE_COUNTERCLOCKWISE_C);
    gr10_30->enGestures(GESTURE_UP | GESTURE_DOWN | GESTURE_LEFT | GESTURE_RIGHT);
    return true;
}

void GR10_30I2CHub::callback()
{
    char res[64] = "";
    if (gr10_30->getDataReady())
    {
        uint16_t gestures = gr10_30->getGesturesState();
        if (gestures & GESTURE_UP)
        {
            // sprintf(res, "\"Gesture\":%s", "\"UP\"");
            sprintf(res, "\"Gesture\":%s", "\"3\"");
        }
        if (gestures & GESTURE_DOWN)
        {
            // sprintf(res, "\"Gesture\":%s", "\"Down\"");
            sprintf(res, "\"Gesture\":%s", "\"4\"");
        }
        if (gestures & GESTURE_LEFT)
        {
            // sprintf(res, "\"Gesture\":%s", "\"Left\"");
            sprintf(res, "\"Gesture\":%s", "\"1\"");
        }
        if (gestures & GESTURE_RIGHT)
        {
            // sprintf(res, "\"Gesture\":%s", "\"Right\"");
            sprintf(res, "\"Gesture\":%s", "\"2\"");
        }
        if (gestures & GESTURE_FORWARD)
        {
            sprintf(res, "\"Gesture\":%s", "\"Forward\"");
        }
        if (gestures & GESTURE_BACKWARD)
        {
            sprintf(res, "\"Gesture\":%s", "\"Backward\"");
        }
        if (gestures & GESTURE_CLOCKWISE)
        {
            sprintf(res, "\"Gesture\":%s", "\"Clockwise\"");
        }
        if (gestures & GESTURE_COUNTERCLOCKWISE)
        {
            sprintf(res, "\"Gesture\":%s", "\"Contrarotate\"");
        }
        if (gestures & GESTURE_WAVE)
        {
            sprintf(res, "\"Gesture\":%s", "\"Wave\"");
        }
        if (gestures & GESTURE_HOVER)
        {
            sprintf(res, "\"Gesture\":%s", "\"Hover\"");
        }
        if (gestures & GESTURE_CLOCKWISE_C)
        {
            sprintf(res, "\"Gesture\":%s", "\"clockwise\"");
        }
        if (gestures & GESTURE_COUNTERCLOCKWISE_C)
        {
            sprintf(res, "\"Gesture\":%s", "\"Continuous counterclockwise\"");
        }
    }
    else
    {
        sprintf(res, "\"Gesture\":%s", "\"None\"");
    }
    data = String(res);
}

bool BME280I2CHub::init()
{
    bme = new DFRobot_BME280_IIC(&Wire1, 0x77);
    name = "BME280I2CHub";
    bme->reset();
    return bme->begin() == DFRobot_BME280::eStatusOK;
}
void BME280I2CHub::callback()
{
    char tempStr[180];
    float temp = bme->getTemperature();
    uint32_t press = bme->getPressure();
    float alti = bme->calAltitude(1015.0, press);
    float humi = bme->getHumidity();
    sprintf(tempStr, "\"Temperature\":%.1f,\"Pressure\":%d,\"Altitude\":%.2f,\"Humidity\":%.1f", temp, press, alti, humi);
    data = String(tempStr);
}

bool URM09I2CHub::init()
{
    URM09 = new DFRobot_URM09();
    name = "URM09I2CHub";
    bool ret = URM09->begin();
    URM09->setModeRange(MEASURE_MODE_AUTOMATIC, MEASURE_RANG_500);
    delay(20);
    return ret;
}
void URM09I2CHub::callback()
{
    char tempStr[64];
    // URM09->measurement();                 // Send ranging command
    float temp = URM09->getTemperature(); // Read temperature
    int16_t dist = URM09->getDistance();  // Read distance
    sprintf(tempStr, "\"UltrasonicSensor\":%d", dist);
    data = String(tempStr);
}

bool ColorI2CHub::init()
{
    tcs = new DFRobot_TCS34725(&Wire1, TCS34725_ADDRESS, TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_1X);
    name = "ColorI2CHub";
    return tcs->begin() == 0;
}
void ColorI2CHub::callback()
{
    char tempStr[64];
    uint16_t clear, red, green, blue;
    tcs->getRGBC(&red, &green, &blue, &clear);
    tcs->lock();
    sprintf(tempStr, "\"R\":%d,\"G\":%d,\"B\":%d", red, green, blue);
    data = String(tempStr);
}

bool AmbientLightI2CHub::init()
{
    name = "AmbientLightI2CHub";
    als = new DFRobot_VEML7700();
    als->begin(); // Init
    return true;
}
void AmbientLightI2CHub::callback()
{
    char tempStr[64];
    float lux;
    als->getALSLux(lux); // Get the measured ambient light value
    sprintf(tempStr, "\"Lux\":%.3f", lux);
    data = String(tempStr);
}

bool TripleAxisAccelerometerI2CHub::init()
{
    name = "TripleAxisAccelerometerI2CHub";
    acce = new DFRobot_LIS2DH12(&Wire1, 0x18);
    bool ret = acce->begin();
    acce->setRange(/*Range = */ DFRobot_LIS2DH12::eLIS2DH12_16g);
    acce->setAcquireRate(/*Rate = */ DFRobot_LIS2DH12::eLowPower_10Hz);
    return ret;
}

void TripleAxisAccelerometerI2CHub::callback()
{
    char tempStr[64];
    long ax, ay, az;
    ax = acce->readAccX(); // Get the acceleration in the x direction
    ay = acce->readAccY(); // Get the acceleration in the y direction
    az = acce->readAccZ(); // Get the acceleration in the z direction
    sprintf(tempStr, "\"x\":%ld,\"y\":%ld,\"z\":%ld", ax, ay, az);
    data = String(tempStr);
}

bool mmWaveI2CHub::init()
{
    name = "mmWaveI2CHub";
    radar = new DFRobot_C4001_I2C(&Wire1, 0x2A);
    bool ret = radar->begin();
    radar->setSensorMode(eExitMode);
    radar->setDetectionRange(/*min*/ 30, /*max*/ 1000, /*trig*/ 300);
    radar->setTrigSensitivity(1);
    radar->setKeepSensitivity(2);
    radar->setDelay(/*trig*/ 100, /*keep*/ 4);
    return ret;
}
void mmWaveI2CHub::callback()
{
    char tempStr[64];
    sprintf(tempStr, "\"motion\":%d", radar->motionDetection() ? 1 : 0);
    data = String(tempStr);
}

bool UVI2CHub::init()
{
    name = "UVI2CHub";
    UVIndex240370Sensor = new DFRobot_UVIndex240370Sensor(&Wire1);
    return UVIndex240370Sensor->begin();
}
void UVI2CHub::callback()
{
    char tempStr[64];
    uint16_t voltage = UVIndex240370Sensor->readUvOriginalData();
    uint16_t index = UVIndex240370Sensor->readUvIndexData();
    sprintf(tempStr, "\"UV\":%d", index);
    data = String(tempStr);
}

bool Bmx160I2CHub::init()
{
    name = "Bmx160I2CHub";
    bmx = new DFRobot_BMX160(&Wire1);
    return bmx->begin();
}

void Bmx160I2CHub::callback()
{
    data = "";
    char tempStr[128];
    sBmx160SensorData_t Omagn, Ogyro, Oaccel;
    bmx->getAllData(&Omagn, &Ogyro, &Oaccel);

    sprintf(tempStr,
            // "\"acc\":{\"x\":%.1f,\"y\":%.1f,\"z\":%.1f}",
            "\"mag\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},"
            "\"gyr\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f},"
            "\"acc\":{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f}",
            Omagn.x, Omagn.y, Omagn.z,
            Ogyro.x, Ogyro.y, Ogyro.z,
            Oaccel.x, Oaccel.y, Oaccel.z
    );
    data = String(tempStr);
}

bool ENS160I2CHub::init()
{
    name = "ENS160I2CHub";
    ens160 = new DFRobot_ENS160_I2C(&Wire1);
    ens160->begin();
    ens160->setPWRMode(ENS160_STANDARD_MODE);
    ens160->setTempAndHum(/*temperature=*/25.0, /*humidity=*/50.0);

    return true;
}

void ENS160I2CHub::callback()
{   
    char tempStr[64];
    
    //uint8_t Status = ens160->getENS160Status();
    //uint8_t AQI = ens160->getAQI();
    uint16_t TVOC = ens160->getTVOC();
    uint16_t ECO2 = ens160->getECO2();

    sprintf(tempStr, "\"ens160_TVOC\":%d,\"ens160_ECO2\":%d",TVOC, ECO2);
    data = String(tempStr);
}

bool MAX30102I2CHub::init()
{
    name = "MAX30102I2CHub";
    max30102 = new DFRobot_BloodOxygen_S_I2C(&Wire1,0x57);

    max30102->begin();
    max30102->sensorStartCollect();
    return true;
}

void MAX30102I2CHub::callback()
{   
    char sop2Str[32], heartRateStr[32];
    static uint8_t skip= 0;
    static uint16_t HeartRate = 0, SPO2 = 0;

    if(skip++ == 10){
        max30102->getHeartbeatSPO2();
        if(max30102->_sHeartbeatSPO2.SPO2 > 0){
            sprintf(sop2Str,"\"max30102_SPO2\":%d",max30102->_sHeartbeatSPO2.SPO2);
            SPO2 = max30102->_sHeartbeatSPO2.SPO2;
        }else{
            sprintf(sop2Str,"\"max30102_SPO2\":%d", SPO2);
        }
        if(max30102->_sHeartbeatSPO2.Heartbeat > 0){
            sprintf(heartRateStr,"\"max30102_HeartRate\":%d",max30102->_sHeartbeatSPO2.Heartbeat);
            HeartRate = max30102->_sHeartbeatSPO2.Heartbeat;
        }else{
            sprintf(heartRateStr,"\"max30102_HeartRate\":%d", HeartRate);
        }
        //sprintf(tempStr, "\"max30102_SPO2\":%d,\"max30102_HeartRate\":%d", max30102->_sHeartbeatSPO2.SPO2, max30102->_sHeartbeatSPO2.Heartbeat);
        data = String(sop2Str + String(",") + heartRateStr);
        //printf("%s",data.c_str());
        //printf("i am here\n");
        skip = 0;
    }
}

bool SCD4XI2CHub::init()
{ 
    name = "SCD4XI2CHub";
    scd4x = new DFRobot_SCD4X(&Wire1);
    scd4x->begin();
    scd4x->enablePeriodMeasure(SCD4X_STOP_PERIODIC_MEASURE);
    scd4x->setTempComp(4.0);
    scd4x->getTempComp();
    scd4x->setSensorAltitude(540);
    scd4x->getSensorAltitude();
    scd4x->enablePeriodMeasure(SCD4X_START_PERIODIC_MEASURE);
    return true;
}

void SCD4XI2CHub::callback()
{
    char tempStr[64];
    if(scd4x->getDataReadyStatus()){
        
        scd4x->readMeasurement(&scd4xData);
        // printf("\"SCD4X\": {\"CO2\": %d, \"Temperature\": %.2f, \"Humidity\": %.2f}\n",
        //        data.CO2ppm, data.temp, data.humidity);
        sprintf(tempStr,"\"scd4x_CO2ppm\":%d", scd4xData.CO2ppm);
        data = String(tempStr);
    }
}

bool BMI160I2CHub::init()
{
    name = "BMI160I2CHub";
    bmi160 = new DFRobot_BMI160;
    if(bmi160->softReset() != BMI160_OK){
        return false;  
    }
    if (bmi160->I2cInit(0x69) != BMI160_OK){
        return false;
    }
    return true;
}

void BMI160I2CHub::callback()
{
    int i = 0;
    int rslt;
    int16_t accelGyro[6]={0};
    char tempStr[128];

    rslt = bmi160->getAccelGyroData(accelGyro);
    if(rslt == 0){
        sprintf(tempStr, "\"bmi160_gyr_x\":%.2f,\"bmi160_gyr_y\":%.2f,\"bmi160_gyr_z\":%.2f,\"bmi160_acc_x\":%.2f,\"bmi160_acc_y\":%.2f,\"bmi160_acc_z\":%.2f",
                accelGyro[0]*3.14/180.0, accelGyro[1]*3.14/180.0, accelGyro[2]*3.14/180.0,
                accelGyro[3]/16384.0, accelGyro[4]/16384.0, accelGyro[5]/16384.0);
        data = String(tempStr);
    }
}