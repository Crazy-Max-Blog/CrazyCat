#pragma once
#include <Arduino.h>

const char GH_upload_portal[] PROGMEM = "<form method='POST' action='/hub/upload_portal' enctype='multipart/form-data'><input type='file' name='update' multiple><input type='submit' value='Upload'></form><br><a href='/'>Home</a>";