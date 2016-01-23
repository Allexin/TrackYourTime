
For building this application, Qt 5 is required

You can get it here: https://www.qt.io/download/

First build TrackYourTime.pro with Qt Creator - it is a typical Qt project

After building do the following:

#Windows

Place TrackYourTime.exe in build folder, copy data folder, 
copy platforms plugin with qwindows, copy necessary qt libs
Pack in compressed folder TrackYourTime_Windows.zip

#Mac OS X

Copy data into TrackYourTime.app/Contents/MacOS
copy platforms/libqcocoa.dylib into TrackYourTime.app/Contents/PlugIns/platforms
add key LSUIElement with value 1 into TrackYourTime.app/Contents/Info.plist
execute macdeployqt with -dmg flag
Rename package to TrackYourTime_MacOSX.dmg

#Linux
Place TrackYourTime in build folder, 
copy data folder, copy checksystem
Pack into TrackYourTime_Linux.tar.gz
