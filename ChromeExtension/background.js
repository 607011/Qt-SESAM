/*
 * Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
 * All rights reserved.
 */

(function(window) {
  var host = 'de.ct.qtsesam';
  var port;
  var currentTabUrl;
  var user = { id: null, pwd: null };

  function getCurrentTabUrl(callback) {
    var queryInfo = {
      active: true,
      currentWindow: true
    };
    currentTabUrl = null;
    chrome.tabs.query(queryInfo, function(tabs) {
      var tab = tabs[0];
      if (typeof tab.url == 'string')
        callback(tab.url);
    });
  }

  function sendMessage() {
    var msg = document.getElementById('msg').value;
    console.log('Sending: ' + JSON.stringify(msg));
    port.postMessage(msg);
  }

  function onMessage(msg) {
    if (msg.cmd === "get-current-tab-url") {
      var reply = { "current-tab-url": currentTabUrl };
      port.postMessage(reply);
    }
    else if (msg.cmd === "set-url") {
      chrome.tabs.update(window.WINDOW_ID_CURRENT, { url: msg.url });
      user.id = msg.userid;
      user.pwd = msg.password;
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
    getCurrentTabUrl(function(url) { currentTabUrl = url; });
    port = chrome.runtime.connectNative(host);
    port.onMessage.addListener(onMessage);
    port.onDisconnect.addListener(onDisconnect);
    chrome.webNavigation.onCompleted.addListener(onCompleted);
  }

  document.addEventListener('DOMContentLoaded', main);

})(window);
