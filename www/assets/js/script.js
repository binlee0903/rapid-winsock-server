let articleNumber = 0;

(function () {
  "use strict";

  // header sticky
  let nav = document.querySelector(".header-nav");
  var lastKnownScrollY = 0;
  var currentScrollY = 0;
  const classes = {
    pinned: "header-nav-pinned",
    unpinned: "header-nav-unpinned",
  };
  function stickyNavigation() {
    if (window.scrollY >= 50) {
      nav.classList.add("header-sticky-top");
    } else {
      nav.classList.remove("header-sticky-top");
    }
  }
  function navbarPinUnpin() {
    currentScrollY = window.pageYOffset;
    if (currentScrollY < lastKnownScrollY) {
      pin();
    } else if (currentScrollY > lastKnownScrollY) {
      if (window.scrollY >= 400) {
        unpin();
      }
    }
    lastKnownScrollY = currentScrollY;
  }
  function pin() {
    if (nav.classList.contains(classes.unpinned)) {
      nav.classList.remove(classes.unpinned);
      nav.classList.add(classes.pinned);
    }
  }
  function unpin() {
    if (
      nav.classList.contains(classes.pinned) ||
      !nav.classList.contains(classes.unpinned)
    ) {
      nav.classList.remove(classes.pinned);
      nav.classList.add(classes.unpinned);
    }
  }
  window.onload = function () {
    window.onscroll = function () {
      navbarPinUnpin();
      stickyNavigation();
    };
  };

  window.addEventListener(
    "load",
    function () {
      // zoom image
      Lightense(".content img");

      // navbar interactions
      window.onscroll = function () {
        navbarPinUnpin();
        stickyNavigation();
      };

      GetArticlesFromServer(0);
    },
    false
  );

  // search-popup
  function searchPopup() {
    let searchBlock = document.querySelector(".search-block");
    let searchOpen = document.querySelectorAll('[data-toggle="search"]');
    let searchClose = document.querySelector('[data-toggle="search-close"]');
    searchOpen.forEach((so) => {
      so.addEventListener("click", function () {
        searchBlock.classList.add("is-visible");
        setTimeout(() => {
          document.querySelector('[aria-label="search-query"]').focus();
        }, 250);
      });
    });
    searchClose.addEventListener("click", function () {
      searchBlock.classList.remove("is-visible");
      document.querySelector('[aria-label="search-query"]').value = "";
      setTimeout(() => {
        document.querySelector("#js-search-results-container").innerHTML = "";
        document.querySelector("#js-search-results-container").style.display =
          "none";
      }, 250);
    });
  }
  searchPopup();

  // menuHumBurger icon toggle Init
  function menuHumBurgerIcon() {
    document
      .querySelector(".navbar-toggler")
      .addEventListener("click", function (e) {
        this.classList.toggle("toggle-menu");
      });
  }
  menuHumBurgerIcon();
})();
