This repository contains all the files related to the server.

List of the files (and short description):
- **.gitrepo**: this directory is a subrepo in the whole repository. It's
    necessary to do it like this in order to push on the server only the server
    related files and not the whole repository.
- **Procfile**: specifies which file to execute in order to start the web
    server (for Heroku).
- **requirements.txt**: list of the Python packages needed by the server.
- **run**: the commands executed by Heroku when a push is done. It starts the
    server (and initialize the database if the relevant line is present).
- **runapp.py**: the python code that starts the server.
- **runtime.txt**: specifies the proper Python version to use. Python-3.5.1 in
    our case.
- **setup.py**: for configuration
- **development.ini**: the configuration file for local tests
- **production.ini**: the configuration file for production (to use on the
    server).
- **tests.ini**: the configuration file for the automatic tests (used for
    continuous integration).
- **makahiya/**: all the Python code of the server.


List of the paths (and function):
- **/**: home page
- **/leds**: display the content of the __leds__ table.
- **/login**: offer a button to log in with google
