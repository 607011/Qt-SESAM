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

(function(window) {

  function main() {
    var statEl = document.getElementById('proxy-connection-status');
    var port = chrome.extension.connect({ name: "c't SESAM" });
    port.postMessage({ status: "ok" });
    port.onMessage.addListener(function(msg) {
      statEl.innerHTML = msg['proxy-connection-status'];
    });
  }

  document.addEventListener('DOMContentLoaded', main);

})(window);
