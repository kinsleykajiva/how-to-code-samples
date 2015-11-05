#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <ctime>
#include <sstream>

#include <grove.h>
#include <grovemoisture.h>
#include <buzzer.h>

#include "../lib/restclient-cpp/include/restclient-cpp/restclient.h"

using namespace std;

// TODO
void notify() {
  if (!getenv("SERVER") || !getenv("AUTH_TOKEN")) {
    std::cerr << "Server not configured." << std::endl;
    return;
  }

  std::time_t now = std::time(NULL);
  char mbstr[sizeof "2011-10-08T07:07:09Z"];
  std::strftime(mbstr, sizeof(mbstr), "%FT%TZ", std::localtime(&now));

  std::stringstream payload;
  payload << "{\"value\":";
  payload << "\"" << mbstr << "\"}";

  RestClient::headermap headers;
  headers["X-Auth-Token"] = getenv("AUTH_TOKEN");

  RestClient::response r = RestClient::put(getenv("SERVER"), "text/json", payload.str(), headers);

  std::cerr << r.code << std::endl;
  std::cerr << r.body << std::endl;
}

// TODO
struct Devices
{
  upm::Buzzer* buzzer;
  upm::GroveMoisture* moisture;

  Devices(){
  };

  // TODO
  void init() {
    // buzzer connected to D5 (digital out)
    buzzer = new upm::Buzzer(5);
    stop_ringing();

    // moisture sensor on analog (A0)
    moisture = new upm::GroveMoisture(0);
  }

  // TODO
  void reset() {
    stop_ringing();
  }

  // TODO
  void alarm() {
    notify();
    message();
    buzzer->playSound(266, 0);
  }

  // TODO
  void message(){
    cout << "Alert! Water is Detected!";
  }

  // TODO
  void stop_ringing() {
    buzzer->stopSound();
    buzzer->stopSound();
  }

  // TODO
  void senseMoisture() {
    for (;;) {
      int val = moisture->value();
      cout << "Moisture value: " << val << ", ";
      if (val >= 0 && val < 300)
          cout << "dry";
      else if (val >= 300 && val < 600)
          cout << "moist";
      else
          cout << "wet";

      cout << endl;

      if (val >= 300)
         alarm();
      else
         stop_ringing();

      cout << endl;
      sleep(1);
    }
  }
};

Devices devices;

// Exit handler for program
void exit_handler(int param)
{
  devices.cleanup();
  exit(1);
}

// The main function for the example program
int main()
{
  // handles ctrl-c or other orderly exits
  signal(SIGINT, exit_handler);

  // check that we are running on Galileo or Edison
  mraa_platform_t platform = mraa_get_platform_type();
  if ((platform != MRAA_INTEL_GALILEO_GEN1) &&
    (platform != MRAA_INTEL_GALILEO_GEN2) &&
    (platform != MRAA_INTEL_EDISON_FAB_C)) {
    std::cerr << "ERROR: Unsupported platform" << std::endl;
    return MRAA_ERROR_INVALID_PLATFORM;
  }

  // create and initialize UPM devices
  devices.init();
  devices.reset();
  devices.senseMoisture();

  return 0;
}
