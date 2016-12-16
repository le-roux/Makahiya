This repository contains all the files related to the server.

List of the files (and short description):
- **requirements.txt**: list of the Python packages needed by the server.
- **setup.py**: for configuration
- **production.ini**: the configuration file for production (to use on the
    server).
- **tests.ini**: the configuration file for the automatic tests (used for
    continuous integration).
- **makahiya/**: all the Python code of the server.


List of the paths (and function):
- **/**: home page
- **/leds**: display the content of the __leds__ table.
- **/login**: offer a button to log in with google
- **/upload**: offers the possibility to upload a mp3 file
- **/file.mp3**: to get the mp3 file
- **/ws/clients/{id}**: websocket between client and server
- **/ws/plants/{id}**: websocket between plant and server
