window.addEventListener("load", async () => {
  const hamburger = document.getElementById("hamburger");
  const menu = document.getElementById("menu");
  const menuItems = Array.from(menu.querySelectorAll("a"));
  const closeMenu = () => {
    menu.classList.add("hidden");
  };
  hamburger.addEventListener("click", () => {
    menu.classList.toggle("hidden");
  });
  menuItems.forEach((item) => {
    item.addEventListener("click", closeMenu);
  });

  document.addEventListener("click", function (event) {
    const targetElement = event.target;

    if (
      !targetElement.closest("#hamburger") &&
      !targetElement.closest("#menu")
    ) {
      menu.classList.add("hidden");
    }
  });
});
