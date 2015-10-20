var LoginManager = (function(window) {

  var port = null;
  var user = {};
  var domain = {};

  var findURL = (function() {
    var Domains = [
          {
            id: /amazon\.de/,
            url: "https://www.amazon.de/ap/signin?_encoding=UTF8&openid.assoc_handle=deflex&openid.claimed_id=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0%2Fidentifier_select&openid.identity=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0%2Fidentifier_select&openid.mode=checkid_setup&openid.ns=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0&openid.ns.pape=http%3A%2F%2Fspecs.openid.net%2Fextensions%2Fpape%2F1.0&openid.pape.max_auth_age=0&openid.return_to=https%3A%2F%2Fwww.amazon.de%2F%3Fref_%3Dnav_signin",
            usr: "#ap_email",
            pwd: "#ap_password",
            frm: "#ap_signin_form"
          },
          {
            id: /(bit\.ly|bitly\.com)/,
            url: "https://bitly.com/a/sign_in",
            usr: "[name=username]",
            pwd: "[name=password]",
            frm: "#sign-in"
          },
          {
            id: /google\.com/,
            url: [ "https://accounts.google.com/ServiceLogin#identifier", "https://accounts.google.com/ServiceLogin#password" ],
            usr: [ "#Email", "#Email-hidden" ],
            pwd: [ "#Passwd-hidden", "#Passwd" ],
            frm: "#gaia_loginform"
          },
          {
            id: /facebook\.com/,
            url: "https://www.facebook.com/login.php",
            usr: "#email",
            pwd: "#pass",
            frm: "#login_form"
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
    port.postMessage(msg);
  }


  chrome.runtime.onConnect.addListener(function tabListener(port) {
    var tab = port.sender.tab;
    port.onMessage.addListener(function(info) {
      console.log("Received from content script: %c%s", "color:green", JSON.stringify(info));
      chrome.tabs.sendMessage(tab.id, { domain: domain, user: user });
      if (info.status === "ok") {
        sendMessageToProxy({ status: "ok", message: info.url + " loaded." })
      }
      else {
        sendMessageToProxy({ status: "error", message: info.url + " could not be loaded." })
      }
    });
  });


  return {
    login: function(url, usr, pwd) {
      user = { id: usr, pwd: pwd };
      domain = findURL(url);
      if (domain.url instanceof Array) {
        sendMessageToProxy({ "status": "error", "message": url + " not supported" });
        return;
      }
      chrome.tabs.query({}, function(tabs) {
        var tabId = null;
        for (var idx in tabs) {
          var tab = tabs[idx];
          if (typeof tab.url === "string" && domain.id.test(tab.url)) {
            tabId = tab.id;
            break;
          }
        }
        if (tabId !== null) {
          chrome.tabs.update(tabId, { active: true, url: domain.url });
        }
        else {
          chrome.tabs.create({ url: domain.url, active: true });
        }
      });
    },
    init: function(msg_port) {
      console.log("LoginManager started.");
      port = msg_port;
    }
  }
})(window);




(function(window) {
  var MessagingHost = 'de.ct.qtsesam';
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
