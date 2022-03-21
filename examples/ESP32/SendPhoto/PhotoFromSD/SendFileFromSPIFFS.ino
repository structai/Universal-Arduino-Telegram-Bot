/*******************************************************************
    A telegram bot for your ESP32 that demonstrates sending an image
    from SD.

    Parts:
    ESP32 D1 Mini stlye Dev board* - http://s.click.aliexpress.com/e/C6ds4my
    (or any ESP32 board)

      = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Example originally written by Vadim Sinitski 

    Library written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBotSenDocu.h>
#include <ArduinoJson.h>



// Wifi network station credentials
#define WIFI_SSID "XXXX"
#define WIFI_PASSWORD "XXXXXXX"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages


unsigned long bot_lasttime;          // last time messages' scan has been done
WiFiClientSecure secured_client;
UniversalTelegramBotSenDocu bot(BOT_TOKEN, secured_client);
    #include <SPIFFS.h>           
    #define FILESYSTEM SPIFFS
File myFile;

bool isMoreDataAvailable()
{
  return myFile.available();
}

byte getNextByte()
{
  return myFile.read();
}


void readFile(fs::FS &fs, String  file_name,bool type)
{
    String path="/"+file_name;
    String chat_id = bot.messages[0].chat_id;
    String sent;
    Serial.printf("Reading file: %s\r\n", path);
    myFile = fs.open(path);
    if(!myFile || myFile.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        return;
    }
    if(type)   sent = bot.sendDocument(chat_id,file_name, "document",myFile.size(),
    isMoreDataAvailable,getNextByte, nullptr, nullptr);
    else sent = bot.sendPhotoByBinary(chat_id,"/new_file.txt", "photo",myFile.size(),
    isMoreDataAvailable,getNextByte, nullptr, nullptr);
    if (sent) 
    {
      Serial.println("was successfully sent");
    } 
    else 
    {
      Serial.println("was not sent");
    }     
    myFile.close();
}
// List all files saved in the selected filesystem
void listDir(const char * dirname, uint8_t levels) 
{
  Serial.printf("Listing directory: %s\r\n", dirname);
  File root = FILESYSTEM.open(dirname, "r");
  if (!root) 
  {
    Serial.println("- failed to open directory\n");
    return;
  }
  if (!root.isDirectory()) 
  {
    Serial.println(" - not a directory\n");
    return;
  }
  File file = root.openNextFile();
  while (file) 
  {
    if (file.isDirectory()) 
    {
      Serial.printf("  DIR : %s\n", file.name());
      if (levels)
        listDir(file.name(), levels - 1);
    }
    else   Serial.printf("  FILE: %s\tSIZE: %d\n", file.name(), file.size());
    file = root.openNextFile();
  }
}
void handleNewMessages(int numNewMessages)
{
  String chat_id = bot.messages[0].chat_id; 
  readFile(SPIFFS,"box.jpg",0);
    Serial.print("box.jpg");
    Serial.print("....\n");
  readFile(SPIFFS,"new_file.txt",1);
    Serial.print("new_file.txt");
    Serial.print("....\n");
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  Serial.print("Initializing SD card....");


  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);// Init filesystem (format if necessary)
  if (!FILESYSTEM.begin()) 
  {
    Serial.println("\nFS Mount Failed.\nFilesystem will be formatted, please wait.");
    #if FORMAT_FS_IF_FAILED
    FILESYSTEM.format();
    ESP.restart();
    #endif
  }
  listDir("/", 0);
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}
