We need to create a series of files to make this thing run. 

The first is the dialog file, which will be set up similarly to that 
in the Oculars plugin. So it will have a .cpp and a .hpp component. These
two components will work as usual (.hpp will set up class definitions, .cpp handles
function definitions and execution)

The second is the connection with the database. We need the database to be accessible
to the plugin somehow. We can make it preloaded and interactive, like they have
in the Oculars plugin, which means we just work with that same PropertyBasedTableModel
function they use. The alternative is to work with an online database, which we can
update as we go along. 

The third is to have a .ui file, written in XML, which will define all of the objects 
that the dialog file can interact with. These interactions I am describing appear at the 
bottom of OcularDialog.cpp, where there are various calls to the pointer "ui". 

NOTE:

The xml file creates objects as QObject classes (like QFrame and QWidget). These can then
be manipulated in the cpp and hpp files by preprocessing the Q object libraries, which
are indexed here:	http://doc.qt.io/qt-5/index.html


We can physically create a list of runs by connecting to a .ini file in the hidden folder ~/.stellarium/modules/MY_PLUGIN


