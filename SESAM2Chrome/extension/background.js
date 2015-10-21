/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

var LoginManager = (function(window) {
  var port, user, domain, loginStep;

  var findURL = (function DomainManager() {
    var Domains = [];
    var xhr = new XMLHttpRequest;
    xhr.onreadystatechange = function(event) {
      if (xhr.readyState !== XMLHttpRequest.DONE)
        return;
      if (xhr.status !== 200)
        return;
      Domains = JSON.parse(xhr.responseText);
      Domains.forEach(function(d) { d.id = new RegExp(d.id); });
    };
    xhr.open("GET", chrome.extension.getURL("/domains.json"));
    xhr.send();

    function parseURI(uri) {
      var parser = document.createElement('a');
      parser.href = uri;
      return parser;
    }

    return function(url) {
      var hostname = parseURI(url).hostname;
      var result = { "url": [ url ], "id": new RegExp(hostname), notfound: true };
      for (var idx in Domains) {
        var d = Domains[idx];
        if (d.id.test(hostname)) {
          result = d;
          break;
        }
      }
      return result;
    };
  })();


  function sendMessageToProxy(msg) {
    if (port !== null) {
      port.postMessage(msg);
    }
    else {
      console.warn("port is null");
    }
  }


  function sendToTab(tabId, msg) {
    if (msg && msg.domain && msg.user) {
      chrome.tabs.sendMessage(tabId, msg);
    }
  }

  chrome.runtime.onConnect.addListener(function tabConnectListener(port) {
    var tab = port.sender.tab;
    port.onMessage.addListener(function tabListener(info) {
      if (domain === null)
        return;
      if (domain.url instanceof Array && loginStep < domain.url.length) {
        sendToTab(tab.id, { domain: domain, user: user, loginStep: loginStep });
      }
      if (info.status === "ok") {
        if (domain.id.test(info.url))
          sendMessageToProxy({ status: "ok", message: info.url + " loaded." })
      }
      else {
        sendMessageToProxy({ status: "error", message: info.url + " could not be loaded." })
      }
      ++loginStep;
    });
  });


  return {
    login: function(url, usr, pwd) {
      loginStep = 0;
      user = { id: usr, pwd: pwd };
      domain = findURL(url);
      if (domain.unsupported) {
        sendMessageToProxy({ status: "error", message: url + " not supported" });
        return;
      }
      sendMessageToProxy({ status: "ok", message: "Found match to " + url + ". Now trying to login to " + domain.url[0] });
      chrome.tabs.query({}, function tabSelector(tabs) {
        var tabId = null;
        for (var idx in tabs) {
          var tab = tabs[idx];
          if (typeof tab.url === "string" && domain.id.test(tab.url)) {
            tabId = tab.id;
            break;
          }
        }
        var url = domain.url[0];
        if (tabId !== null) {
          chrome.tabs.update(tabId, { active: true, url: url });
        }
        else {
          chrome.tabs.create({ url: url, active: true });
        }
      });
    },
    init: function initialize(msg_port) {
      console.log("LoginManager started.");
      port = msg_port;
      user = null;
      domain = null;
      loginStep = 0;
    }
  }
})(window);



(function(window) {
  var MessagingHost = 'de.ct.dev.qtsesam';
  var port = null;

  function onMessage(msg) {
    if (msg.cmd === "login") {
      LoginManager.login(msg.url, msg.userId, msg.userPwd);
    }
    else {
      // XXX: simple echo for debugging purposes
      port.postMessage(msg);
    }
  }

  function onDisconnect() {
    console.warn('Disconnected. ' + chrome.runtime.lastError.message);
    port = null;
  }

  function main() {
    console.log("%c c't SESAM %c - This Chrome extension gets remotely controlled by Qt-SESAM & Co.",
                "background-color: #0061af; color: white; font-weight: bold; letter-spacing: 5px",
                "color: #0061af; font-weight: bold;");
    console.log("%cCopyright (c) 2015 Oliver Lau, Heise Medien GmbH & Co. KG. All rights reserved.",
                "color: #aaa");
    port = chrome.runtime.connectNative(MessagingHost);
    port.onMessage.addListener(onMessage);
    port.onDisconnect.addListener(onDisconnect);

    LoginManager.init(port);

    chrome.extension.onConnect.addListener(function popupListener(popupPort) {
      popupPort.onMessage.addListener(function(msg) {
        popupPort.postMessage({ "proxy-connection-status": port !== null ? "connected" : "disconnected" });
      });
    });

  }

  document.addEventListener('DOMContentLoaded', main);

})(window);
