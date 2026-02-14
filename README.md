# Clock Radio 2.0

### This project is a upgrade to the original FM Radio project. It adds a clock to the radio, making it a more versatile device. The device displays the time on a LED dot matrix display while  the radio is playing in the background. The radio can be controlled using a rotary encoder, which allows the user to change the volume and tune the radio stations.<br>

### The core components of the project is built using an Arduino Nano, a TEA5767 FM radio module, a DS3231 RTC module, a MAX7219 LED dot matrix display, and a rotary encoder.<br> 

### To get started wit the invididual components I recommend going over these articles - [TEA5767 FM Radio Module](https://www.instructables.com/Arduino-TEA5767-FM-Radio-Receiver/), [DS3231 RTC Module](https://lastminuteengineers.com/ds3231-rtc-arduino-tutorial/), [MAX7219 LED Dot Matrix Display](https://lastminuteengineers.com/max7219-dot-matrix-arduino-tutorial/), [Rotary Encoder](https://arduinogetstarted.com/tutorials/arduino-rotary-encoder)

### Additional improvements to this project include the use of filter capacitors to reduce noise in the audio output, pull up resistors for the serial data and clock lines on the nano. The code is also optimized to reduce the number of libraries used, which helps to reduce the overall size of the code and improve performance. The compelte connection diagram can be seen below - made in Fritzing 
 ![Circuit Diagram](<FM CLOCK RADIO 2.0_bb.png>)

### A demo of the project can be seen in the video below -
[![Clock Radio 2.0 Demo](https://img.youtube.com/vi/9n8Xo5s1l3o/0.jpg)](https://www.youtube.com/watch?v=-Bv6NTO21fA)

### In the future, I plan to build a custom PCB for this project, which will help to further reduce the size and improve the overall design of the device. Ideally I would also not power everything using the 5V and GND pins on the nano and would improve on the design to use an external power source. I also plan to 3D print a custom enclosure for the device, which will help to protect the components and give it a more polished look.


### Overall, this project was a fun and rewarding experience, and I am excited to continue improving it in the future. It is a great example of how different components can be integrated together to create a functional and useful device.