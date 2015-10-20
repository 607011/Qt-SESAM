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

    sendResponse({ status: "ok", openedByExtension: true })
  }

  chrome.runtime.onMessage.addListener(function(msg, sender, sendResponse) {
    doLogin(msg.domain, msg.user, sendResponse);
  });

  var portToExtension = chrome.runtime.connect();
  portToExtension.postMessage({ title: document.title, url: document.location.href, status: "ok", openedByExtension: false });

})($, window);
