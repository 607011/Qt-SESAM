var LoginManager = (function(window) {
  var port = null;
  var Domains = [
        {
          "id": "amazon.de",
          "url": "https://www.amazon.de/ap/signin?_encoding=UTF8&openid.assoc_handle=deflex&openid.claimed_id=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0%2Fidentifier_select&openid.identity=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0%2Fidentifier_select&openid.mode=checkid_setup&openid.ns=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0&openid.ns.pape=http%3A%2F%2Fspecs.openid.net%2Fextensions%2Fpape%2F1.0&openid.pape.max_auth_age=0&openid.return_to=https%3A%2F%2Fwww.amazon.de%2F%3Fref_%3Dnav_signin",
          "usr": "ap_email",
          "pwd": "ap_password",
          "frm": "ap_signin_form"
        }
      ];
  var user = { id: null, pwd: null };
  var domain = { id: null, url: null, usr: null, pwd: null, frm: null };


  function tabReady(tab) {
    console.log("tabReady()", tab);
  }


  function parseURI(uri) {
    var parser = document.createElement('a');
    parser.href = uri;
    return parser;
  }


  function findURL(url) {
    var result = null;
    var hostname = parseURI(url).hostname;
    for (var idx in Domains) {
      var d = Domains[idx];
      if (hostname.indexOf(d.id) >= 0) {
        result = d;
        break;
      }
    }
    return result;
  }


  chrome.runtime.onConnect.addListener(function tabListener(port) {
    var tab = port.sender.tab;
    port.onMessage.addListener(function(info) {
      console.log("Received from content script: %c%s", "color:green", JSON.stringify(info));
      chrome.tabs.sendMessage(tab.id, { domain: domain, user: user });
    });
  });


  return {
    login: function(url, usr, pwd) {
      user = { id: usr, pwd: pwd };
      domain = findURL(url);
      if (domain === null) {
        port.postMessage({ "status": "error", "message": url + " not supported" });
        return;
      }
      chrome.tabs.query({}, function(tabs) {
        var tabId = null;
        for (var idx in tabs) {
          var tab = tabs[idx];
          if (typeof tab.url === "string" && tab.url.indexOf(domain.id) === 0) {
            tabId = tab.id;
            break;
          }
        }
        if (tabId !== null) {
          chrome.tabs.update(tabId, { active: true, url: domain.url }, tabReady);
        }
        else {
          chrome.tabs.create({ url: domain.url, active: true }, tabReady);
        }
      });
    },
    init: function(msg_port) {
      console.log("LoginManager started.");
      port = msg_port;
    }
  }
})(window);
