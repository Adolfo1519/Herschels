# Herschels
This repository is dedicated to working on the Herschels in a Box project. The goal of the project
is to create a Stellarium plugin which can run the various exploratory sweeps the Herschels did 
(starting with Caroline and William, though we might add John). 

# Version information
This was built for Stellarium 9.0 and has not been tested on newer versions. Same goes for this documentation.

# Download instructions

1. Download Stellarium into /home/{{user}}/StelDev/stellarium
   1. `cd /home/{{user}}/StelDev`
   1. `svn co https://stellarium.svn.sourceforge.net/svnroot/stellarium/trunk/extmodules`
1. Download this repo
   1. `git clone https://github.com/Adolfo1519/Herschels`
   1. `cd Herschels`

# Installation instructions

1. Copy the plugin folder (Herschels) to the stellarium dev directory you've set up
   1. `cp -r Herschels /home/{{user}}/StelDev/stellarium/plugins`
1. Our plugin needs a specific telescope and lens setting installed into the Oculars plugin, on which it depends. 2 options here:
   1. Overwrite the Oculars default file prior to installation
      1. `cp default_ocular.ini /home/{{user}}/StelDev/stellarium/plugins/Oculars/`
   1. Take a look at those settings and use the stellarium gui to add them in the GUI after installation
1. Create a build directory and build the app from the altered source code
   1. `export STELROOT=/home/{{user}}/StelDev/stellarium`
   1. `cd /home/{{user}}/StelDev/stellarium`
   1. `mkdir -p builds/macosx`
   1. `cd builds/macosx`
   1. `cmake ../..`
   1. `make`
1. Optional: make the mac app
   1. to delete from installations: `rm -rf Stellarium.app`
   1. `make install`
   1. `make mac_app`
1. Launch the app (/home/{{user}}/StelDev/stellarium/builds/macosx/src/stellarium executable)
1. Open plugins dashboard from left-hand toolbar
   1. Make sure your Herschel oculars exist and are properly configured
   1. Set Herschels to launch at startup
1. Quit, and restart the app
   1. The Herschels plugin will now be available on the bottom toolbar 
   
# Sweep data

* sweeps_data.csv is the transcription of the sweeps
* formatSweepFile.py takes the data in this csv and creates default_sweep.ini
* ini_to_db.py imports the data into herschels_data.db

# Updating sweep dataset:

1. Update sweeps_data.csv
1. Run formatSweepFile.py
1. Overwrite the file in /home/{{user}}/StelDev with the resulting ini file
1. Delete the Herschel plugin application defaults for Stellarium, for example:
   1. `rm -rf ~/Library/Application\ Support/Stellarium/modules/Herschels`
1. Run through the installation instructions again, including checking and resetting the appropriate application default settings