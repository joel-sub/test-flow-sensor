const serialDataElement = document.getElementById('serial-data');

function updateSerialData() {
  fetch('https://192.168.1.31:80')
    .then(response => response.text())
    .then(data => {
      serialDataElement.textContent = data;
    });
}

setInterval(updateSerialData, 3000);
