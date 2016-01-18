#Windows
Place TrackYourTime.exe in build folder, copy data folder, copy platforms plugin with qwindows, copy necessary qt libs

#Mac OS X
copy data into TrackYourTime.app/Contents/
add key LSUIElement with value 1 into TrackYourTime.app/Contents/Info.plist
process macdeployqt with -dmg flag


#Linux
Place TrackYourTime in build folder, copy data folder, copy checksystem