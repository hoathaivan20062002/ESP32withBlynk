/* Điền vào Template ID của bạn ( Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID "TMPL6Hvve0kKA"
#define BLYNK_TEMPLATE_NAME "IOT controller ESP32"
#define BLYNK_AUTH_TOKEN "sglbMgWeSFAvnxRg4UTUKUMTBSHrrY_t"
// Thông tin WiFi của bạn.
// Đặt mật khẩu là "" để sử dụng mạng mở.
char ssid[] = "P2408";
char pass[] = "244466666";


#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Preferences.h>
#include <AceButton.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

using namespace ace_button;
Preferences pref;

#define DHTPIN            23 //D23   chân kết nối với DHT

#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321


#define SCREEN_WIDTH 128 // Chiều rộng màn hình OLED, tính bằng pixel
#define SCREEN_HEIGHT 64 // Chiều cao màn hình OLED, tính bằng pixel
#define OLED_RESET     -1 // Chân reset # (hoặc -1 nếu chia sẻ chân reset của Arduino)
#define SCREEN_ADDRESS 0x3C ///<  Xem tài liệu để biết Địa chỉ; 0x3D cho 128x64, 0x3C cho 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Giá trị Setpoint và setHumi (đơn vị độ Celsius)
float setTemp = 0;
float setHumi = 0;
float currentTemp = 0;
float currentHumi = 0;



// Định nghĩa GPIO kết nối với Relays và Buttons
#define RelayPin1 18  //D18
#define RelayPin2 19  //D19

#define ButtonPin1 25  //D25
#define ButtonPin2 26  //D26 
#define ButtonPin3 27  //D27

#define wifiLed   2   //D2

// Thay đổi các virtual pin theo blynk
#define VPIN_Text           V0
#define VPIN_Mode           V1
#define VPIN_currentTemp    V2
#define VPIN_currentHumi    V3
#define VPIN_setTemp        V4
#define VPIN_setHumi        V5
#define VPIN_Heater         V6
#define VPIN_Humidifier     V7

// Trạng thái Relays và Mode
bool heaterState = LOW; //Định nghĩa biến để ghi nhớ trạng thái chuyển đổi cho bơi lạnh
bool humidifierState = LOW; //Định nghĩa biến để ghi nhớ trạng thái chuyển đổi cho Bơi ẩm
bool modeState = LOW; //Định nghĩa biến để ghi nhớ mode

int wifiFlag = 0;


char auth[] = BLYNK_AUTH_TOKEN;

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);

void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);


BlynkTimer timer;
DHT dht(DHTPIN, DHTTYPE);

// Khi nút App được nhấn - chuyển trạng thái

BLYNK_WRITE(VPIN_Heater) {
  heaterState = param.asInt();
  digitalWrite(RelayPin1, !heaterState);
  pref.putBool("Heater", heaterState);
}

BLYNK_WRITE(VPIN_Humidifier) {
  humidifierState = param.asInt();
  digitalWrite(RelayPin2, !humidifierState);
  pref.putBool("Humidifier", humidifierState);
}

BLYNK_WRITE(VPIN_Mode) {
  modeState = param.asInt();
  pref.putBool("Mode", modeState);
}

BLYNK_WRITE(VPIN_setTemp) {
  setTemp = param.asFloat();
  pref.putBool("setemp", setTemp);
}

BLYNK_WRITE(VPIN_setHumi) {
  setHumi = param.asFloat();
  pref.putBool("Humidity", setHumi);
}

void checkBlynkStatus() { // được gọi  sau 2 giây bởi Simpletimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 2);
    digitalWrite(wifiLed, LOW);
  }
  if (isconnected == true) {
    wifiFlag = 0;
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 2);
    display.println(" Blynk IoT Connected ");
    digitalWrite(wifiLed, HIGH);
    Blynk.virtualWrite(VPIN_Text, "IoT Controller With ESP32");
  }
  display.display();
  delay(1000);
}

BLYNK_CONNECTED() {
// Cập nhật trạng thái mới nhất lên server
  Blynk.virtualWrite(VPIN_Text, "IoT Temperature Controller with ESP32");
  Blynk.virtualWrite(VPIN_Mode, modeState);
  Blynk.syncVirtual(VPIN_currentTemp);
  Blynk.syncVirtual(VPIN_currentHumi);
  Blynk.syncVirtual(VPIN_setTemp);
  Blynk.syncVirtual(VPIN_setHumi);
  Blynk.virtualWrite(VPIN_Heater, heaterState);
  Blynk.virtualWrite(VPIN_Humidifier, humidifierState);

}

void setup()
{
  Serial.begin(115200);
// Cập nhật trạng thái 
  pref.begin("Relay_State", false);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  pinMode(ButtonPin1, INPUT_PULLUP);
  pinMode(ButtonPin2, INPUT_PULLUP);
  pinMode(ButtonPin3, INPUT_PULLUP);

  
// SSD1306_SWITCHCAPVCC = tạo điện áp màn hình từ 3.3V nội bộ
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Không tiếp tục, lặp vô hạn
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

// Trong quá trình khởi động, tất cả các relay nên TẮT
  digitalWrite(RelayPin1, !heaterState);
  digitalWrite(RelayPin2, !humidifierState);

  dht.begin();    
// Kích hoạt cảm biến DHT
  digitalWrite(wifiLed, LOW);

  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);

  button1.init(ButtonPin1);
  button2.init(ButtonPin2);
  button3.init(ButtonPin3);

  //Blynk.begin(auth, ssid, pass);
  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus); // Kiểm tra xem máy chủ Blynk đã kết nối mỗi 2 giây
  timer.setInterval(1000L, sendSensor); // Gửi dữ liệu cảm biến đến Đám mây Blynk mỗi 1 giây
  Blynk.config(auth);
  delay(1000);

  getRelayState(); // Lấy trạng thái cuối cùng của Relay và Thiết lập giá trị của Nhiệt độ & Độ ẩm

  Blynk.virtualWrite(VPIN_Heater, heaterState);
  Blynk.virtualWrite(VPIN_Humidifier, humidifierState);
  Blynk.virtualWrite(VPIN_setTemp, setTemp);
  Blynk.virtualWrite(VPIN_setHumi, setHumi);
}


void readSensor() {

  currentTemp = dht.readTemperature();
  currentHumi = dht.readHumidity();
  if (isnan(currentHumi) || isnan(currentTemp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

void sendSensor()
{
  readSensor();
// Bạn có thể gửi bất kỳ giá trị nào vào bất kỳ thời điểm nào.
// Vui lòng không gửi nhiều hơn 10 giá trị mỗi giây.
  Blynk.virtualWrite(VPIN_Text, "IoT Temperature Controller");
  Blynk.virtualWrite(VPIN_currentTemp, currentTemp);
  Blynk.virtualWrite(VPIN_currentHumi, currentHumi);
}

void getRelayState()
{
  //Serial.println("reading data from NVS");
  modeState = pref.getBool("Mode", 0);
  Blynk.virtualWrite(VPIN_Mode, modeState);
  delay(200);
  heaterState = pref.getBool("Heater", 0);
  digitalWrite(RelayPin1, !heaterState);
  Blynk.virtualWrite(VPIN_Heater, heaterState);
  delay(200);
  humidifierState = pref.getBool("Humidifier", 0);
  digitalWrite(RelayPin2, !humidifierState);
  Blynk.virtualWrite(VPIN_Humidifier, humidifierState);
  delay(200);
  setTemp = pref.getBool("setemp", 0);
  Blynk.virtualWrite(VPIN_setTemp, setTemp);
  delay(200);
  setHumi = pref.getBool("Humidity", 0);
  Blynk.virtualWrite(VPIN_setHumi, setHumi);
  delay(200);
}


void DisplayData()  {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 7);
  display.println("----------");
  display.setCursor(2, 20);
  display.print(int(currentTemp));
  display.print((char)247);
  display.println("C");
  display.setCursor(60, 20);
  display.print(": ");
  display.print(int(currentHumi));
  display.println("%");
  display.setCursor(0, 34);
  display.println("----------");

  display.setTextSize(1);
  display.setCursor(0, 57);
  display.print("sTemp:");
  display.print(int(setTemp));
  display.print((char)247);
  display.println("C");
  display.setTextSize(1);
  display.setCursor(67, 57);
  display.print("sHumi:");
  display.print(int(setHumi));
  display.println("%");


  if (modeState == 1) {
    display.setTextSize(1);
    display.setCursor(20, 45);
    display.print("Automatic Mode");
    if (currentTemp < setTemp) {
      heaterState = 1;
      digitalWrite(RelayPin1, !heaterState);
      pref.putBool("Heater", heaterState);
      Serial.println("Heater ON");
      Blynk.virtualWrite(VPIN_Heater, heaterState);
    } else {
      heaterState = 0;
      digitalWrite(RelayPin1, !heaterState);
      pref.putBool("Heater", heaterState);
      Serial.println("Heater OFF");
      Blynk.virtualWrite(VPIN_Heater, heaterState);
    } if (currentHumi < setHumi) {
      humidifierState = 1;
      digitalWrite(RelayPin2, !humidifierState);
      Serial.println("Humidifier ON");
      pref.putBool("Humidifier", humidifierState);
      Blynk.virtualWrite(VPIN_Humidifier, humidifierState);
    } else {
      humidifierState = 0;
      digitalWrite(RelayPin2, !humidifierState);
      Serial.println("Humidifier OFF");
      pref.putBool("Humidifier", humidifierState);
      Blynk.virtualWrite(VPIN_Humidifier, humidifierState);
    }
  } else {
    display.setTextSize(1);
    display.setCursor(32, 45);
    display.print("Manual Mode");
    
  }
  display.display();
}


void loop()
{
  Blynk.run();
  timer.run();
  DisplayData();
  button1.check();
  button2.check();
  button3.check();
}

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Mode");
  switch (eventType) {
    case AceButton::kEventReleased:
      modeState = !modeState;
      pref.putBool("Mode", modeState);
      Blynk.virtualWrite(VPIN_Mode, modeState);
      break;
  }
}
void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Heater");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin1, heaterState);
      heaterState = !heaterState;
      pref.putBool("Heater", heaterState);
      Blynk.virtualWrite(VPIN_Heater, heaterState);
      break;
  }
}
void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Humidifier");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin2, humidifierState);
      humidifierState = !humidifierState;
      pref.putBool("VPIN_Humidifier", humidifierState);
      Blynk.virtualWrite(VPIN_Humidifier, humidifierState);
      break;
  }
}