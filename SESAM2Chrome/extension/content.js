(function(window) {
  function doLogin(domain, user) {
    console.log("domain: %c%s", "color:green", JSON.stringify(domain));
    console.log("user: %c%s", "color:green", JSON.stringify(user));

    var usrEl = document.getElementById(domain.usr);
    if (usrEl === null) {
      sendResponse({ status: "error", message: "user input element not found" });
      return;
    }
    usrEl.value = user.id;

    var pwdEl = document.getElementById(domain.pwd);
    if (pwdEl === null) {
      sendResponse({ status: "error", message: "password input element not found" });
      return;
    }
    pwdEl.value = user.pwd;

    var frmEl =  document.getElementById(domain.frm);
    frmEl.submit();
    sendResponse({ status: "ok" })

  }

  chrome.runtime.onMessage.addListener(function(msg, _, sendResponse) {
    doLogin(msg.domain, msg.user);
  });

  chrome.runtime.connect().postMessage({ title: document.title, url: document.location.href, status: "ok" });

})(window);
