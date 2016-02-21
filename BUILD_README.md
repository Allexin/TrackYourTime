
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
execute macdeployqt with -dmg flag(<path_to_qt/macdeployqt TrackYourTime.app -dmg>)
Rename package to TrackYourTime_MacOSX.dmg

#Linux
Place TrackYourTime in build folder, 
copy data folder, copy checksystem
Pack into TrackYourTime_Linux.tar.gz


#Localization  
its possible to make this work from console without QtCreator.  
But QtCreator is simple and powerfull tool and i prefer to use it.  

Open project in QtCreator  
Go to menu Tools->External->Linguist->Update Translations(lupdate)  
Launch Qt Linguist(<qt_path/bin/linguist>)  
Open *.ts file for edit  
Edit file(edit item and press Ctrl+Enter)
Save file  
Go to menu Tools->External->Linguist->Release Translations(lrelease)  
Move *.qm file into data/languages

If you create new language  - create file with name data/languages/lang_<lang_id>_name.utf8  
Open this file in utf8 text editor  
And place only one line in utd8 encoding - name of language in native format  
