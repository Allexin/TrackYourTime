# TrackYourTime 

#[Documentation](https://github.com/Allexin/TrackYourTime/wiki/User-Manual) [[en](https://github.com/Allexin/TrackYourTime/wiki/User-Manual)|[ru](https://github.com/Allexin/TrackYourTime/wiki/%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D1%82%D0%B5%D0%BB%D1%8F)]  
[![Join the chat at https://gitter.im/Allexin/TrackYourTime](https://badges.gitter.im/Allexin/TrackYourTime.svg)](https://gitter.im/Allexin/TrackYourTime?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)  

#Downloads
[Latest Stable Version](https://github.com/Allexin/TrackYourTime/releases/tag/0.9.2)

#Roadmap

https://github.com/Allexin/TrackYourTime/wiki

# Installation

Remember! App will work from the folder where you first launch it. If you want to change the app location - move app folder into a new location, launch, go to Settings and press apply. This will change autostart settings to new location.

## Windows
Extract archive with windows app in any folder and launch TrackYourTime.exe.

## Max OS X
Extract archive with Mac OS X app, move TrackYourTime.app to Applications and launch

## Linux

Prerequisites:

* Windows Manager with `_NET_ACTIVE_WINDOW` property supported
* udev input system
* application access to `/dev/input/by-id/*`

Extract archive with Linux app in any folder and launch `checksystem` from app folder. Follow `checksystem` instructions to resolve problems. After that launch TrackYourTime app.

Sometimes app autorun may not work in your environment. In this case manually add application link to autorun. Do not forget to set working dir to app dir. Application can run without it, but icons and localizations will not work properly.
