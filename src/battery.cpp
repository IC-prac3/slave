// ----------------------------------------------------------------------------------
// Initilizes BQ24195L_PMIC to allow battery operation and recharge
// ----------------------------------------------------------------------------------

#include <Arduino_PMIC.h>

bool init_PMIC()
{
  bool error = false;
  
  if (!PMIC.begin()) {
      error = true;
    return false;
  }

  // Set the input current limit to 1 A and the overload input voltage to 3.88 V
  if (!PMIC.setInputCurrentLimit(2.0)) { 
    error = true;
  }

  if (!PMIC.setInputVoltageLimit(3.88)) {
    digitalWrite(LED_BUILTIN, HIGH);
    error = true;
  }

  // set the minimum voltage used to feeding the module embed on Board
  if (!PMIC.setMinimumSystemVoltage(3.5)) {
    error = true;
  }

  // Set the desired charge voltage to 4.2 V
  if (!PMIC.setChargeVoltage(4.2)) {
    error = true;
  }

  // Set the charge current to 375 mA
  // the charge current should be defined as maximum at (C for hour)/2h
  // to avoid battery explosion (for example for a 750 mAh battery set to 0.375 A)
  if (!PMIC.setChargeCurrent(0.375)) {
    error = true;
  }

  if (!PMIC.enableCharge()) {
    error = true;
  }
  
  PMIC.isBattConnected();
  PMIC.isPowerGood();
  PMIC.getChargeCurrent();
  PMIC.getChargeVoltage();
  PMIC.getMinimumSystemVoltage();
  PMIC.canRunOnBattery();
  
  if (error) {
      digitalWrite(LED_BUILTIN, HIGH);
    return false;
  }
  else return true;
}
