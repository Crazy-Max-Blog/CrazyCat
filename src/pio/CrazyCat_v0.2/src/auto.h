bool tenz_bool = 0;

void auto_begin() {

}

void auto_tick() {
  static gh::Timer tmr(50);
  if(tmr) {
    static bool tenz_bb=0;
    if((tenz_valuet>sets.devs.tenz.min_mass)!=tenz_bb) {tenz_bool = (tenz_valuet>sets.devs.tenz.min_mass); tenz_bb=tenz_bool; ghub_led = tenz_bool; hub.sendUpdate("led"); led_state=tenz_bool;}
    hub.update("mass").valueInt(tenz_valuet);
  }
}