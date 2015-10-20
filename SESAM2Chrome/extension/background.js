/*
 * Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
 * All rights reserved.
 */

(function(window) {
  var host = 'de.ct.qtsesam';
  var port;
  var currentTabUrl;
  var user = { id: null, pwd: null };

  function sendMessage() {
    var msg = document.getElementById('msg').value;
    console.log('Sending: ' + JSON.stringify(msg));
    port.postMessage(msg);
  }

  function tabReady(tab) {
    tabId = tab.id;
    var reply = { status: "ok", url: tab.url };
    port.postMessage(reply);
  }

  function onMessage(msg) {
    if (msg.cmd === "login") {
      var openTabs = [];
      user.id = msg.userid;
      user.pwd = msg.password;
      chrome.tabs.query({}, function(tabs) {
        var tabId = window.WINDOW_ID_CURRENT;
        var tabUrl = msg.url;
        for (var i in tabs) {
          var tab = tabs[i];
          if (typeof tab.url === "string" && tab.url.indexOf(msg.url) === 0) {
            tabUrl = tab.url;
            tabId = tab.id;
            break;
          }
        }
        if (tabUrl === null) {
          chrome.tabs.create({ url: tabUrl, active: true }, tabReady);
        }
        else {
          chrome.tabs.update(tabId, { active: true, url: tabUrl }, tabReady);
        }
      });
    }
    else {
      // XXX: simple echo for debugging purposes
      port.postMessage(msg);
    }
  }

  function onDisconnect() {
    console.log('Disconnected. ' + chrome.runtime.lastError.message);
    port = null;
  }

  function onCompleted(details)
  {
    // TODO: implement login automagic with credentials in `user` object
  }


  function main() {
    console.log("%c c't SESAM %c - This Chrome extensions gets remotely controlled by Qt-SESAM et al.",
                "background: #0061AF; color: white",
                "background: transparent; color: #0061AF");
    console.log("%cCopyright (c) 2015 Oliver Lau, Heise Medien GmbH & Co. KG. All rights reserved.",
                "background: transparent; color: #aaa");
    LoginManager.init();
    port = chrome.runtime.connectNative(host);
    port.onMessage.addListener(onMessage);
    port.onDisconnect.addListener(onDisconnect);
    chrome.webNavigation.onCompleted.addListener(onCompleted);
  }

  document.addEventListener('DOMContentLoaded', main);

})(window);
