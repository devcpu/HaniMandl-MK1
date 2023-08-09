#include <Arduino.h>
#include <ESPFS.h>

#ifdef ESP32
#include <SPIFFS.h>
#else
#error This works on ESP32 only
#endif

boolean ESPFSInit() {
  bool initok = false;
  initok = SPIFFS.begin();
  if (!(initok)) { // Format SPIFS, of not formatted. - Try 1
    log_i("Format SPIFFS (try 1). Please wait ... ");
    SPIFFS.format();
    initok = SPIFFS.begin();
  }
  if (!(initok)) { // Format SPIFS, of not formatted. - Try 2
    log_w("Format SPIFFS (try 2). Please wait ... ");
    SPIFFS.format();
    initok = SPIFFS.begin();
  }
  if (!initok) {
    log_e("SPIFFS not OK");
  } else {
    log_i("ESP FS ready");
  }
  // #endif
  return initok;
}
