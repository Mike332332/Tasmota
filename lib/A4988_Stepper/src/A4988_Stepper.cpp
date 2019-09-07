/* This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Drives a bipolar motor, controlled by A4988 stepper driver circuit
 */

#include "Arduino.h"
#include "A4988_Stepper.h"
#include <stdlib.h>
A4988_Stepper::A4988_Stepper( int   m_spr
                          , int   m_rpm
                          , short m_mis
                          , short m_dir_pin
                          , short m_stp_pin
                          , short m_ena_pin
                          , short m_ms1_pin
                          , short m_ms2_pin
                          , short m_ms3_pin ) {
  last_time     = 0; // time stamp in us of the last step taken
  motor_SPR     = m_spr; // StepsPerRevolution
  motor_RPM     = m_rpm; // RoundsPerMinute
  motor_MIS     = m_mis; // Microsteps w/o effect if MS1-MS3 not connected - then full steps anyway
  motor_dir_pin = m_dir_pin;
  motor_stp_pin = m_stp_pin;
  motor_ena_pin = m_ena_pin;
  motor_ms1_pin = m_ms1_pin;
  motor_ms2_pin = m_ms2_pin;
  motor_ms3_pin = m_ms3_pin;

  adjustDelay();
  adjustPins();
  adjustMicrosteps();
}

void A4988_Stepper::adjustPins(void) {
   // setup the pins on the microcontroller:
   pinMode(motor_dir_pin, OUTPUT);
   pinMode(motor_stp_pin, OUTPUT);
   if (motor_ena_pin <99) {
     pinMode(motor_ena_pin, OUTPUT);
     digitalWrite(motor_ena_pin, HIGH);
   }

   if ((motor_ms1_pin<99)&&(motor_ms2_pin<99)&&(motor_ms3_pin<99)) {
     pinMode(motor_ms1_pin, OUTPUT);
     pinMode(motor_ms2_pin, OUTPUT);
     pinMode(motor_ms3_pin, OUTPUT);
   }
}

void A4988_Stepper::adjustMicrosteps() {
   if ((motor_ms1_pin<99)&&(motor_ms2_pin<99)&&(motor_ms3_pin<99)) {
     switch (motor_MIS){
       case 1:
         digitalWrite(motor_ms1_pin, LOW);
         digitalWrite(motor_ms2_pin, LOW);
         digitalWrite(motor_ms3_pin, LOW);
       break;
       case 2:
         digitalWrite(motor_ms1_pin, HIGH);
         digitalWrite(motor_ms2_pin, LOW);
         digitalWrite(motor_ms3_pin, LOW);
       break;
          digitalWrite(motor_ms1_pin, LOW);
          digitalWrite(motor_ms2_pin, HIGH);
          digitalWrite(motor_ms3_pin, LOW);
       case 4:
          digitalWrite(motor_ms1_pin, HIGH);
          digitalWrite(motor_ms2_pin, HIGH);
          digitalWrite(motor_ms3_pin, LOW);
       break;
       case 8:
          digitalWrite(motor_ms1_pin, LOW);
          digitalWrite(motor_ms2_pin, LOW);
          digitalWrite(motor_ms3_pin, HIGH);
       break;
       case 16:
          digitalWrite(motor_ms1_pin, HIGH);
          digitalWrite(motor_ms2_pin, HIGH);
          digitalWrite(motor_ms3_pin, HIGH);
       break;
     }
   } else {
     motor_MIC = 1;
   }
}

 void A4988_Stepper::adjustDelay(void) {
   motor_delay = 60L * 1000L * 1000L / motor_SPR / motor_RPM / motor_MIC;
 }

void A4988_Stepper::setMIS(short oneToSixteen) {
   motor_MIS = oneToSixteen;
   adjustMicrosteps();
 }

 short A4988_Stepper::getMIS(void) {
     return motor_MIS;
 }

 void A4988_Stepper::setRPM(int howManyRounds) {
   motor_RPM = howManyRounds;
   adjustDelay();
 }

 int A4988_Stepper::getRPM(void) {
   return motor_RPM;
 }

void A4988_Stepper::setSPR(int howManySteps){
  motor_SPR = howManySteps;
  adjustDelay();
}

int A4988_Stepper::getSPR(void) {
  return motor_SPR;
}

void A4988_Stepper::enable(){
  if (motor_ena_pin < 99) {digitalWrite(motor_ena_pin, LOW);}
}

void A4988_Stepper::disable(){
  if (motor_ena_pin < 99) {digitalWrite(motor_ena_pin, HIGH);}
}

void A4988_Stepper::doMove(long howManySteps)
{
  long steps_togo = abs(howManySteps);  // how many steps to take
  bool lastStepWasHigh = false;
  digitalWrite(motor_dir_pin, howManySteps>0?LOW:HIGH);
  enable();
  while (steps_togo > 0) {
    delay(0); // don't get watchdoged in loop
    unsigned long now = micros();
    // move if delay has passed:
    if (now - last_time >= motor_delay) {
      digitalWrite(motor_stp_pin, lastStepWasHigh?LOW:HIGH);
      lastStepWasHigh = !lastStepWasHigh;
      // remeber step-time if last signal was HIGH we can pull low after 50ms as only HIGH actually moves the stepper
      last_time = lastStepWasHigh?now-50:now;
      if (!lastStepWasHigh) steps_togo--; // same here - only HIGH moves, if pulled LOW step is completed...
    }
  }
  disable();
}

void A4988_Stepper::doRotate(long howManyDegrees)
{ long  lSteps = 0;
  lSteps = motor_SPR*motor_MIS*howManyDegrees/360;
  doMove(lSteps);
}

void A4988_Stepper::doTurn(float howManyTimes)
{ long lSteps = 0;
  lSteps = howManyTimes*motor_SPR;
  doMove(lSteps);
}

int A4988_Stepper::version(void)
{
  return 1;
}
