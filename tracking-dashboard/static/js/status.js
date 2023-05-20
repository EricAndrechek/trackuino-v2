const getStatus = async () => {
  // TODO: add error handling
  const response = await fetch("/api/tunnel");
  // get status code
  const status = await response.status;
  let data = {};
  // get response body
  try {
    data = await response.json();
    data["status"] = status;
  } catch (e) {
    if (status == 200 && e instanceof SyntaxError) {
      data = {
        error: "Server Offline",
        status: status,
      };
    } else {
      data = {
        error: e,
        status: status,
      };
    }
    console.log(e);
  }
  return data;
};

const getStatusHTML = (status) => {
  let icon, colorClass, message;

  switch (status) {
    case "healthy":
      icon = "fa-check-circle";
      colorClass = "status-healthy";
      message =
        "Healthy - The tunnel is active and serving traffic through four connections to the Cloudflare global network.";
      break;
    case "degraded":
      icon = "fa-exclamation-triangle";
      colorClass = "status-degraded";
      message =
        "Degraded - The tunnel is active and serving traffic, but at least one individual connection has failed. Further degradation in tunnel availability could risk the tunnel going down and failing to serve traffic.";
      break;
    case "down":
      icon = "fa-times-circle";
      colorClass = "status-down";
      message =
        "Down - The tunnel cannot serve traffic as it has no connections to the Cloudflare global network.";
      break;
    case "inactive":
      icon = "fa-info-circle";
      colorClass = "status-inactive";
      message =
        "Inactive - This value is reserved for tunnels which have been created, but have never been run.";
      break;
    default:
      icon = "fa-question-circle";
      colorClass = "status-unknown";
      message = "Unknown Status";
  }

  let html = `
<span class="text-md font-semibold ${colorClass}" title="${message}">
  <i class="fas ${icon} icon"></i>
  ${status.toUpperCase()}
</span>
`;

  return html;
};

const createBoxes = (data) => {
  const container = document.getElementById("container");
  const existingBoxes = Array.from(container.getElementsByClassName("box"));

  // if status is not 200, show error message
  if (data.status != 200 || data.error) {
    // edit box if it already exists
    const existingBox = container.querySelector(`[id="error-box"]`);
    if (existingBox) {
      const status = existingBox.querySelector("div");
      status.innerHTML = `
        <i class="fas fa-exclamation-triangle icon"></i>
        &nbsp;${data.error}
      `;
      return;
    }
    const errorBox = document.cloneNode(true).getElementById("box-template");
    errorBox.id = "error-box";
    errorBox.innerHTML = `
      <div class="text-md font-semibold status-error">
        <i class="fas fa-exclamation-triangle icon"></i>
        ${data.error}
      </div>
    `;
    container.appendChild(errorBox);
    return;
  }

  // Remove boxes that no longer exist
  existingBoxes.forEach((box) => {
    const shouldRemove = !data.result.find(
      (tunnel) => tunnel.name === box.dataset.name
    );
    if (shouldRemove) {
      container.removeChild(box);
    }
  });

  // Add or update existing boxes
  data.result.forEach((tunnel) => {
    const existingBox = container.querySelector(`[data-name="${tunnel.name}"]`);
    if (existingBox) {
      const status = existingBox.querySelector("div");
      status.innerHTML = getStatusHTML(tunnel.status);
    } else {
      const box = document.cloneNode(true).getElementById("box-template");
      box.removeAttribute("id");
      box.dataset.name = tunnel.name;

      const title = document.cloneNode(true).getElementById("box-title-template");
      title.removeAttribute("id");
      title.innerText = tunnel.name;

      box.appendChild(title);

      const details = tunnel.details.ingress || [];
      details.forEach((detail) => {
        if (detail.hostname) {
          const url = document.cloneNode(true).getElementById("box-url-template");
          url.removeAttribute("id");
          let url_value = detail.hostname;
          if (detail.path) {
            url_value += "/" + detail.path;
          }
          url.innerHTML = `<a href="https://${url_value}" target="_blank">${url_value}</a>`;
          box.appendChild(url);
        }
      });

      const status = document.cloneNode(true).getElementById("box-status-template");
      status.removeAttribute("id");
      status.innerHTML = getStatusHTML(tunnel.status, tunnel.config);

      box.appendChild(status);
      container.appendChild(box);
    }
  });
};

const updateBoxes = async () => {
  const data = await getStatus();
  createBoxes(data);
};

window.addEventListener("load", async () => {
  await updateBoxes(); // Initial box update on page load
  setInterval(updateBoxes, 5000); // Update boxes every 5 seconds (adjust the interval as needed)
});
