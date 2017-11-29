var tabsCounters = {}

function getCurrentTabUrl(callback) {
  var queryInfo = {
    active: true,
    lastFocusedWindow: true
  };

  chrome.tabs.query(queryInfo, function(tabs) {
    var tab = tabs[0];
	if (tab == null) {
		var url = "undefined";
        var id = -1;
	} else {
		var url = tab.url;
        var id = tab.id;
	}
    console.assert(typeof url == 'string', 'tab.url should be a string');

    callback(url,id);
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

var delimeter = ":";

function sendState(url,id){
	var TRACKER_INFO='PREFIX=TYTET&VERSION=1&APP_1=chrome.exe&APP_2=chromium.exe&APP_3=Google-chrome-stable&APP_4=google-chrome&APP_5=chromium-browser&APP_6=Google%20Chrome&STATE='+extractDomain(url);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "http://127.0.0.1:25856?"+TRACKER_INFO, true);
    xhr.send();     
    
    if (typeof tabsCounters[id] === 'undefined'){
        tabsCounters[id] = 0
    }
    var tabTime = tabsCounters[id] + 1
    tabsCounters[id] = tabTime
    
    
    var m = Math.floor(tabTime / 60).toString() ;
    var s = (tabTime % 60).toString();
    if (s.length==1) s = "0"+s;
    var text = m+delimeter+s;
    chrome.browserAction.setBadgeText({text:text});
    if (delimeter==":")
        delimeter = " ";
    else
        delimeter = ":";
}

function prepareData(){
    getCurrentTabUrl(sendState);
}

setInterval(prepareData, 1000);
chrome.browserAction.setBadgeBackgroundColor({color:[0,0,0,255]});
