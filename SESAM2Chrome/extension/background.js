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
    var Domains = [
          {
            id: /amazon\.de/,
            url: [ "https://www.amazon.de/ap/signin?_encoding=UTF8&openid.assoc_handle=deflex&openid.claimed_id=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0%2Fidentifier_select&openid.identity=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0%2Fidentifier_select&openid.mode=checkid_setup&openid.ns=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0&openid.ns.pape=http%3A%2F%2Fspecs.openid.net%2Fextensions%2Fpape%2F1.0&openid.pape.max_auth_age=0&openid.return_to=https%3A%2F%2Fwww.amazon.de%2F%3Fref_%3Dnav_signin" ],
            usr: [ "#ap_email" ],
            pwd: [ "#ap_password" ],
            frm: [ "#ap_signin_form" ]
          },
          {
            id: /(bit\.ly|bitly\.com)/,
            url: [ "https://bitly.com/a/sign_in" ],
            usr: [ "[name=username]" ],
            pwd: [ "[name=password]" ],
            frm: [ "#sign-in" ]
          },
          {
            id: /facebook\.com/,
            url: [ "https://www.facebook.com/login.php" ],
            usr: [ "#email" ],
            pwd: [ "#pass" ],
            frm: [ "#login_form" ]
          },
          {
            id: /github\.com/,
            url: [ "https://github.com/login" ],
            usr: [ "#login_field" ],
            pwd: [ "#password" ],
            btn: [ "input[type=submit]" ]
          },
          {
            id: /gmx\.net/,
            url: [ "https://www.gmx.net/" ],
            usr: [ "#inpLoginFreemailUsername" ],
            pwd: [ "#inpLoginFreemailPassword" ],
            // frm: [ "[name=loginForm]" ],
            btn: [ "input[type=submit]" ],
            unsupported: true
          },
          {
            id: /google\.com/,
            url: [ "https://accounts.google.com/ServiceLogin#identifier", "https://accounts.google.com/ServiceLogin#password" ],
            usr: [ "#Email", null ],
            pwd: [ null, "#Passwd" ],
            btn: [ "#next", "#signIn" ]
          },
          {
            id: /paypal\.com/,
            url: [ "https://www.paypal.com/signin" ],
            usr: [ "#email" ],
            pwd: [ "#password" ],
            frm: [ "[name=login]" ],
            btn: [ "#btnLogin" ]
          }
        ];

    return function(url) {
      var hostname = parseURI(url).hostname;
      var result = { url: url, id: new RegExp(hostname) };
      for (var idx in Domains) {
        var d = Domains[idx];
        if (d.id.test(hostname)) {
          result = d;
          break;
        }
      }
      return result;
    }
  })();


  function parseURI(uri) {
    var parser = document.createElement('a');
    parser.href = uri;
    return parser;
  }


  function sendMessageToProxy(msg) {
    if (port !== null)
      port.postMessage(msg);
    else
      console.warn("port is null");
  }


  function sendToTab(tabId, msg) {
    console.log("sendToTab() loginStep = %d", loginStep, msg);
    chrome.tabs.sendMessage(tabId, msg,
                            function responseCallback(response) {
                              ++loginStep;
                              console.log(response);
                              msg.loginStep = loginStep;
                              if (response.status === "ok") {
                                if (loginStep < domain.url.length) {
                                  // dirty hack to allow multi-step logins à la Google
                                  setTimeout(function() { sendToTab(tabId, msg); }, 500);
                                }
                              }
                              else {
                                // TODO
                              }
                            });
  }


  chrome.runtime.onConnect.addListener(function tabConnectListener(port) {
    var tab = port.sender.tab;
    port.onMessage.addListener(function tabListener(info) {
      console.log("tabListener() loginStep = %d", loginStep);
      if (info.status === "ok") {
        sendMessageToProxy({ status: "ok", message: info.url + " loaded." })
      }
      else {
        sendMessageToProxy({ status: "error", message: info.url + " could not be loaded." })
      }
    });
    console.log("tabConnectListener() loginStep = %d", loginStep);
    if (domain === null || loginStep < domain.url.length) {
      sendToTab(tab.id, { domain: domain, user: user, loginStep: loginStep });
    }
  });


  return {
    login: function(url, usr, pwd) {
      loginStep = 0;
      user = { id: usr, pwd: pwd };
      domain = findURL(url);
      if (domain.unsupported) {
        sendMessageToProxy({ "status": "error", "message": url + " not supported" });
        return;
      }
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
    console.log('Disconnected. ' + chrome.runtime.lastError.message);
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
  }

  document.addEventListener('DOMContentLoaded', main);

})(window);
