/*
 * Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
 * All rights reserved.
 */

(function(window) {
  var host = 'de.ct.qtsesam';
  var port = null;
  var currentTabUrl = null;

  function getCurrentTabUrl(callback) {
    var queryInfo = {
      active: true,
      currentWindow: true
    };
    chrome.tabs.query(queryInfo, function(tabs) {
      var tab = tabs[0];
      var url = tab.url;
      console.assert(typeof url == 'string', 'tab.url must be a string');
      callback(url);
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
    }
    else if (msg.cmd === "login") {
      // TODO: implement login automagic
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

  function main() {
    getCurrentTabUrl(function(url) { currentTabUrl = url; });
    port = chrome.runtime.connectNative(host);
    port.onMessage.addListener(onMessage);
    port.onDisconnect.addListener(onDisconnect);
    document.getElementById('send-button').addEventListener('click', sendMessage);
  }

  document.addEventListener('DOMContentLoaded', main);

})(window);
