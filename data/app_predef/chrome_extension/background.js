function getCurrentTabUrl(callback) {
  var queryInfo = {
    active: true,
    lastFocusedWindow: true
  };

  chrome.tabs.query(queryInfo, function(tabs) {
    var tab = tabs[0];
    var url = tab.url;
    console.assert(typeof url == 'string', 'tab.url should be a string');

    callback(url);
  });
}

function extractDomain(url) {
    var domain;
    //find & remove protocol (http, ftp, etc.) and get domain
    if (url.indexOf("://") > -1) {
        domain = url.split('/')[2];
    }
    else {
        domain = url.split('/')[0];
    }

    //find & remove port number
    domain = domain.split(':')[0];

    return domain;
}


function sendState(url){
    var TRACKER_INFO='PREFIX=TYTET;VERSION=1;APP_1=chrome.exe;APP_2=chromium.exe;APP_3=Google-chrome-stable;APP_4=google-chrome;APP_5=chromium-browser;STATE='+extractDomain(url);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "http://127.0.0.1:25856?data=["+TRACKER_INFO+"]", true);
    xhr.send();
    
}

function prepareData(){
    getCurrentTabUrl(sendState);
}

chrome.alarms.onAlarm.addListener(prepareData);
chrome.alarms.create("TRACK_YOUR_TIME_TIMER", {
       delayInMinutes: 0.05, periodInMinutes: 0.05}
              );