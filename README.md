## Summary

Development of a thermostat with an MSP432P401R MCU. The main goal of this project is to learn some Design patterns explained in the book "Design patterns for embedded systems in C" from Bruce Powel Douglass.

## Sensors and actuators

The sensors and actuators used so far in the project are these:
* DHT11 - Temperature and humidity sensor.
* HD44780U - 16x2 LCD screen with an I2C module embedded. 

## Class diagram

*To be done*

## Threads

### Display_console thread

* Reads data to be printed from a queue
* if queue is not empty, prints all its data
* else, sched_yield()

### Temperature_reading thread

* Read temperature from DHT11 sensor
    * Avoid yielding execution while reading!.
* Store data in a queue
* Sleep for at least 15 seconds
    * Minimum time available between readings.
    
### Display_LCD thread

* Reads temp data from a queue
* Prints data
* sched_yield()

### Keypad thread

* while loop that checks for flags to be set when keypad buttons are pressed
    * Keypad buttons generates interrupts.
* updates shared data structures accordingly (increase/decrease target temp, set a timer...)
    * shared DS should implement access control mechanisms (mutex, semaphores,...)
    
### Temperature_control thread

* Checks if heating has to be turned on or off
* It activates or deactivates a relay connected to a GPIO
* Periodically checks if sensed_temp < target_temp
    * True -> close circuit
    * False -> open circuit
* sleep when finishes
