/*
 * Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
 * All rights reserved.
 */

(function(window) {
  var MessagingHost = 'de.ct.qtsesam';
  var port = null;

  function sendMessage() {
    var msg = document.getElementById('msg').value;
    console.log('Sending: ' + JSON.stringify(msg));
    port.postMessage(msg);
  }

  function onMessage(msg) {
    if (msg.cmd === "login") {
      console.log("%c%s", "color:red", JSON.stringify(msg));
      LoginManager.login(msg.url, msg.userId, msg.userPwd);
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
    console.log("%c c't SESAM %c - This Chrome extensions gets remotely controlled by Qt-SESAM & Co.",
                "background-color: #0061af; color: white; font-weight: bold;",
                "color: #0061af; font-weight: bold;");
    console.log("%cCopyright (c) 2015 Oliver Lau, Heise Medien GmbH & Co. KG. All rights reserved.",
                "color: #aaa");
    port = chrome.runtime.connectNative(MessagingHost);
    port.onMessage.addListener(onMessage);
    port.onDisconnect.addListener(onDisconnect);

    LoginManager.init(port);
  }

  document.addEventListener('DOMContentLoaded', main);

})(window);
