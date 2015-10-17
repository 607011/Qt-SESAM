/*
 * Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
 * All rights reserved.
 */

(function(window) {
  var host = 'de.ct.qtsesam';
  var port = null;

  function getCurrentTabUrl(callback) {
    var queryInfo = {
      active: true,
      currentWindow: true
    };
    chrome.tabs.query(queryInfo, function(tabs) {
      var tab = tabs[0];
      var url = tab.url;
      console.assert(typeof url == 'string', 'tab.url should be a string');
      callback(url);
    });
  }
  
  function sendMessage() {
    var msg = { text: document.getElementById("msg").value };
    console.log('Sending : ""' + msg.text + '"');
    port.postMessage(msg);
  }
  
  function main() {
    getCurrentTabUrl(function(url) {
      document.getElementById('status').innerHTML = "current tab url: " + url;
    });
    port = chrome.runtime.connectNative(host);
    console.log(port);
    port.onMessage.addListener(function(msg) {
      document.getElementById('output').value += JSON.stringify(msg) + "\n";
    });
    port.onDisconnect.addListener(function() {
      console.log('Disconnected. ' + chrome.runtime.lastError.message);
      port = null;
    });
    document.getElementById('send-button').addEventListener('click', sendMessage);
  }

  document.addEventListener('DOMContentLoaded', main);

})(window);
