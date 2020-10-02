#include <mbed.h>
#include "DHT.h"
#include "rtos.h"
#include "EthernetInterface.h"

DHT dht(A0, SEN51035P); // Temprature and Humidity sensor
EthernetInterface net;

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

void httpPOST(char *path, char *body)
{
  char sbuffer[512];
  // Bring up the ethernet interface
  net.connect();

  // Show the network address
  SocketAddress a;
  net.get_ip_address(&a);
  printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

  // Open a socket on the network interface, and create a TCP connection to mbed.org
  TCPSocket socket;
  socket.open(&net);

  // net.gethostbyname("163.172.146.229", &a);
  a.set_ip_address("163.172.146.229");
  a.set_port(1337);
  socket.connect(a);
  // Send a simple http request
  printf("%s\r\n", path);
  printf("%s\r\n", body);
  printf("%d\r\n\r\n", sizeof body);

  sprintf(sbuffer, "POST %s HTTP/1.1\r\nHost: 163.172.146.229\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", path, sizeof body, body);
  int scount = socket.send(sbuffer, sizeof sbuffer);
  printf("%s\r\n", sbuffer);
  printf("sent %d [%.*s]\n", scount, strstr(sbuffer, "\r\n") - sbuffer, sbuffer);

  // Recieve a simple http response and print out the response line
  char rbuffer[512];
  int rcount = socket.recv(rbuffer, sizeof rbuffer);
  printf("%s\r\n", rbuffer);
  printf("recv %d [%.*s]\n", rcount, strstr(rbuffer, "\r\n") - rbuffer, rbuffer);

  // Close the socket to return its memory and bring down the network interface
  socket.close();

  // Bring down the ethernet interface
  net.disconnect();
  printf("Done\n");
}

int main(void)
{
  float temperature;
  float humidity;
  char postBody[64];
  char postPath[32];

  getTemperatureHumidity(temperature, humidity);

  net.connect();
  sprintf(postBody, "{\"mac\":\"%s\"}", net.get_mac_address());
  net.disconnect();
  sprintf(postPath, "/devices");

  httpPOST(postPath, postBody);

  while (1)
  {

    getTemperatureHumidity(temperature, humidity);

    // printf("Temperature is %.0f \r\n", temperature);
    // printf("Humidity is %.0f \r\n", humidity);
  }
}