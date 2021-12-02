#include "uMQTTBroker.h"

class myMQTTBroker: public uMQTTBroker
{
public:

    int tmst = 0;
    int buzz = 800;

    virtual bool onConnect(IPAddress addr, uint16_t client_count) {
      Serial.println(addr.toString()+" connected");
      return true;
    }
    
    virtual void onData(String topic, const char *data, uint32_t length) {
      char data_str[length+1];
      os_memcpy(data_str, data, length);
      // Copia el largo de caracteres a la nueva memoria mapeada para dest, en este caso el string, el del medio es el dato en si.
      data_str[length] = '\0';
      //Serial.println("received topic '"+topic+"' with data '"+(String)data_str+"'");
      if(topic == "tmst/buzz"){
        buzz = atoi(data_str);
      }
      if(topic == "tmst/move"){
        tmst = atoi(data_str);
      }
      else{
        tmst = 0;
      }
    }
    
/*  virtual void setup()
    {
      myBroker.init();
      myBroker.subscribe("#");
    }
    
    int counter = 0;
    
    virtual void loop()
    {
      Publish the counter value as String
      myBroker.publish("broker/counter", (String)counter++);
      delay(1000);
    } 
    */
};

myMQTTBroker myBroker;
