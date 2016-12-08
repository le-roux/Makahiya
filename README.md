Makahiya - ROSE project 2016/2017
Author: Sylvain LE ROUX & Tanguy ROUFFIGNAC

Structure:
- ChibiOS_16.1.5/: directory containing the ChibiOS code (writtent by Giovanni
        Di Sirio).
- olimexP407/: directory containing the code specific to the project and that
    runs on the development board.
- code/: directory containing code related to the project but that doesn't run
    on the development board but on a laptop.
- test/: automatic tests

What's working now:
- The detection algorithm for touches and slides can be found in the code/
    directory. Related tests are on the test/ directory.
- The audio driver is working. It's located in the olimexP407/ directory.
    It's able to read a mp3 file from the serial link (serial over USB), to
    decode it on the microprocessor and to send it to the codec that will
    play it. Plug your earphones on the jack plug to hear your music.
