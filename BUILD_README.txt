#Windows
Place TrackYourTime.exe in build folder, copy data folder, copy platforms plugin with qwindows, copy necessary qt libs
Pack int TrackYourTime_Windows.zip

#Mac OS X
copy data into TrackYourTime.app/Contents/MacOS
copy platforms/libqcocoa.dylib into TrackYourTime.app/Contents/PlugIns/platforms
add key LSUIElement with value 1 into TrackYourTime.app/Contents/Info.plist
process macdeployqt with -dmg flag
Rename package to TrackYourTime_MacOSX.dmg


#Linux
Place TrackYourTime in build folder, copy data folder, copy checksystem
Pack into TrackYourTime_Linux.tar.gz