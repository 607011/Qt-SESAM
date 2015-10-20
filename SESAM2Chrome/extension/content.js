(function($, window) {
  function doLogin(domain, user, sendResponse) {
    var usrEl = $(domain.usr);
    if (usrEl === null) {
      sendResponse({ status: "error", message: "user input element not found" });
      return;
    }
    usrEl.val(user.id);

    var pwdEl = $(domain.pwd);
    if (pwdEl === null) {
      sendResponse({ status: "error", message: "password input element not found" });
      return;
    }
    pwdEl.val(user.pwd);

    var frmEl =  $(domain.frm);
    frmEl.submit();

    sendResponse({ status: "ok" })
  }

  chrome.runtime.onMessage.addListener(function(msg, _, sendResponse) {
    if (typeof msg.domain.url !== "object")
      doLogin(msg.domain, msg.user, sendResponse);
  });

  chrome.runtime.connect().postMessage({ title: document.title, url: document.location.href, status: "ok" });

})($, window);
