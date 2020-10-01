#include <mbed.h>
#include "DHT.h"
#include "rtos.h"
#include "http_request.h"
#include "network-helper.h"

DHT dht(A0, SEN51035P); // Temprature and Humidity sensor

void getTemperatureHumidity(float &temperature, float &humidity)
{
  int err = 1;
  while (err != 0)
  {
    ThisThread::sleep_for(2000);
    err = dht.readData();
  }
  temperature = dht.ReadTemperature(CELCIUS);
  humidity = dht.ReadHumidity();
}

int main(void)
{
  NetworkInterface *network = connect_to_default_network_interface();

  if (!network)
  {
    printf("Cannot connect to the network, see serial output\n");
    return 1;
  }

  while (1)
  {
    float temperature;
    float humidity;

    getTemperatureHumidity(temperature, humidity);

    printf("Temperature is %.0f \r\n", temperature);
    printf("Humidity is %.0f \r\n", humidity);
    {
      HttpRequest *post_req = new HttpRequest(network, HTTP_POST, "http://httpbin.org/post");
      post_req->set_header("Content-Type", "application/json");

      const char body[] = "{\"hello\":\"world\"}";

      HttpResponse *post_res = post_req->send(body, strlen(body));
      if (!post_res)
      {
        printf("HttpRequest failed (error code %d)\n", post_req->get_error());
        return 1;
      }

      printf("\n----- HTTP POST response -----\n");
      dump_response(post_res);

      delete post_req;
    }
  }
}