//Vcc---Vcc
//GND---GND
//CS--- D8 / 1k ohm resistor
//Reset---D1
//D/C--- D2 /1k ohm resistor
//SDI/mosi---D7 / 1k ohm resistor
//SCK---D5 / 1k ohm resistor
//LED---Vcc

#include <ESP8266WiFi.h>
/* 依赖 PubSubClient 2.4.0 */
#include <PubSubClient.h>
/* 依赖 ArduinoJson 5.13.4 */
#include <ArduinoJson.h>

#include <UTFT.h>
#include <SPI.h>
#define SENSOR_PIN    13


#include <SimpleDHT.h>
int pinDHT11=16;
SimpleDHT11 dht11(pinDHT11);
byte temperature = 0;
byte humidity = 0;
/* 修改1 ------------------------------------------ */
/* 连接您的WIFI SSID和密码 */
#define WIFI_SSID         "Hold"
#define WIFI_PASSWD       "0987654321"
/* 修改1 end--------------------------------------- */
 
/* 修改2 ------------------------------------------ */
/* 设备证书信息*/
#define PRODUCT_KEY       "a1rlMv14XfF"
#define DEVICE_NAME       "dvaaaa"
#define DEVICE_SECRET     "1SsaV3Vshu0XtR4jH7RQfGjWwiP29Pvx"
#define REGION_ID         "cn-shanghai"
/* 修改2 end--------------------------------------- */
 
/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER       PRODUCT_KEY ".iot-as-mqtt." REGION_ID ".aliyuncs.com"
#define MQTT_PORT         1883
#define MQTT_USRNAME      DEVICE_NAME "&" PRODUCT_KEY
 
/* 修改3 ------------------------------------------ */
#define CLIENT_ID    "esp8266|securemode=3,signmethod=hmacsha1,timestamp=1234567890|"
// 请使用以上说明中的加密工具或参见MQTT-TCP连接通信文档加密生成password。
// 加密明文是参数和对应的值（clientIdesp8266deviceName${deviceName}productKey${productKey}timestamp1234567890）按字典顺序拼接
// 密钥是设备的DeviceSecret
#define MQTT_PASSWD       "1ED64C744526F79A8A1D67828B6EAE501942CCFC"
/* 修改3 end--------------------------------------- */
 
#define ALINK_BODY_FORMAT         "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
 
unsigned long lastMs = 0;
WiFiClient espClient;
PubSubClient  client(espClient);

//***显示屏start***//
// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];


// Modify the line below to match your display and wiring:
UTFT myGLCD ( ST7735, 15, 5, 4 );
//***显示屏end***//
void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.println((char *)payload);
 
}
 
 
void wifiInit()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }
 
    Serial.println("Connected to AP");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
 
    
Serial.print("espClient [");
 
 
    client.setServer(MQTT_SERVER, MQTT_PORT);   /* 连接WiFi之后，连接MQTT服务器 */
    client.setCallback(callback);
}
 
 
void mqttCheckConnect()
{
    while (!client.connected())
    {
        Serial.println("Connecting to MQTT Server ...");
        if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))
 
        {
 
            Serial.println("MQTT Connected!");
 
        }
        else
        {
            Serial.print("MQTT Connect err:");
            Serial.println(client.state());
            delay(5000);
        }
    }
}
 
 
void mqttIntervalPost()
{
    char param[32];
    char jsonBuf[128];

/* 修改4 ------------------------------------------ */
    sprintf(param, "{\"mtemp\":%d,\"mhumi\":%d}", (int)temperature,(int)humidity);
 
/* 修改4 end--------------------------------------- */
 
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    Serial.print("上传数据");
    Serial.println(d);
}
void Display(int x,int y,int index,double temp,double hum)
{
//    myGLCD.setColor ( 144, 30, 255 );
//    myGLCD.setFont(BigFont);//设置字体大小
//    myGLCD.print ( String("PM2.5"), x, y );
//    myGLCD.setFont(SmallFont);//设置字体大小
//    myGLCD.setColor ( 144, 255, y );
//    myGLCD.printNumF(pm,2,100,y+3);
//    myGLCD.print ( String("mg"), 135, y+3 );
    
    myGLCD.setColor ( 144, 30, 255 );
    myGLCD.setFont(BigFont);//设置字体大小
    myGLCD.print ( String("Temp"), x, y+index );
    myGLCD.setFont(SmallFont);//设置字体大小
    myGLCD.setColor ( 144, 255, 45 );
    myGLCD.printNumF(temp,1,100,y+index+3);
    myGLCD.print ( String("C"), 135,y+index+3 );
    
    myGLCD.setColor ( 144, 30, 255 );
    myGLCD.setFont(BigFont);//设置字体大小
    myGLCD.print ( String("Hum"), 10, y+index+index );
    myGLCD.setFont(SmallFont);//设置字体大小
    myGLCD.setColor ( 144, 255, 60 );
    myGLCD.printNumF(hum,1,100,y+index+index+3);
    myGLCD.print ( String("RH%"), 135, y+index+index+3 );
}
 
void setup() 
{
 
    pinMode(SENSOR_PIN,  INPUT);
    /* initialize serial for debugging */
    Serial.begin(115200);
    Serial.println("Demo Start");
 
    wifiInit();

    //显示屏start//
     // Setup the LCD
    myGLCD.InitLCD (  );
    myGLCD.setFont ( SmallFont );
      int buf[158];
    int x, x2;
    int y, y2;
    int r;
  
    double temp=25.5;
    double hum=41.1;
    // Clear the screen and draw the frame
    myGLCD.clrScr (  );
    myGLCD.setColor ( 255, 0, 0 );
    myGLCD.fillRect ( 0, 0, 159, 13 );
    myGLCD.setColor ( 64, 64, 64 );
    myGLCD.fillRect ( 0, 114, 159, 127 );
    myGLCD.setColor ( 255, 255, 255 );
    myGLCD.setBackColor ( 255, 0, 0 );
    myGLCD.print ( String("Smart Home"), CENTER, 1 );
    myGLCD.setBackColor ( 64, 64, 64 );
    myGLCD.setColor ( 255, 255, 0 );
    myGLCD.print ( String("Mr.Aimen"), LEFT+1, 114 );
    myGLCD.print ( String("(C)2015"), RIGHT, 114 );
    myGLCD.setColor ( 0, 0, 255 );
    myGLCD.drawRect ( 0, 13, 159, 113 );
    //显示屏end//
    myGLCD.setBackColor ( 0, 0, 0 );
    
    Display(10,30,20,temp,hum);
}   

// the loop function runs over and over again forever
void loop()
{
    if (millis() - lastMs >= 100000)
    {
        lastMs = millis();
        mqttCheckConnect(); 
        /* 上报消息心跳周期 */
        mqttIntervalPost();
    }
 
    client.loop();

   
    //TODO 读取数据显示
  
     int err = SimpleDHTErrSuccess;
     if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
        Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
        return;
      }
      Serial.print("Sample OK: ");
      Serial.print((int)temperature); Serial.print(" *C, "); 
      Serial.print((int)humidity); Serial.println(" H");
       Display(10,30,20,(int)temperature,(int)humidity);
      // DHT11 sampling rate is 1HZ.
      delay(2000);
}
