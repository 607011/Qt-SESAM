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

(function($, window) {
  var response = { title: document.title, url: document.location.href, status: "ok" };
  var portToExtension = chrome.runtime.connect();
  portToExtension.postMessage(response);

  window.addEventListener("hashchange", function() {
    portToExtension.postMessage({ event: "hashchange", url: document.location });
  });

  function doLogin(msg, _, sendResponse) {
    var domain = msg.domain;
    var user = msg.user;
    var loginStep = msg.loginStep;

    if (domain.usr[loginStep]) {
      var usrEl = $(domain.usr[loginStep]);
      if (usrEl === null) {
        sendResponse({ status: "error", message: "user input element not found" });
        return;
      }
      usrEl.val(user.id);
    }

    if (domain.pwd[loginStep]) {
      var pwdEl = $(domain.pwd[loginStep]);
      if (pwdEl === null) {
        sendResponse({ status: "error", message: "password input element not found" });
        return;
      }
      pwdEl.val(user.pwd);
    }

    if (domain.btn && domain.btn[loginStep]) {
      var btnEl = $(domain.btn[loginStep]);
      if (btnEl === null) {
        sendResponse({ status: "error", message: "submit button not found" });
        return;
      }
      btnEl.trigger("click");
    }
    else if (domain.frm && domain.frm[loginStep]) {
      var frmEl = $(domain.frm[loginStep]);
      if (frmEl === null) {
        sendResponse({ status: "error", message: "form element not found" });
        return;
      }
      frmEl.submit();
    }

    response.domain = domain;
    response.user = user;
    response.loginStep = loginStep;
    sendResponse(response);
  }

  chrome.runtime.onMessage.addListener(doLogin);


})($, window);
