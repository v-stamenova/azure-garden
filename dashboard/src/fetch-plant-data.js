async function fetchLatestData() {
  try {
    const response = await axios.get('http://localhost:3010/api/last');
    const data = response.data;
    const dataPanel = document.getElementById('data-panel');

    // Destructure the data
    const { timestamp, temperature, humidity, soilMoisture } = data;

    console.log(data);
    // Format the timestamp
    const formattedTimestamp = new Date(timestamp).toLocaleString();
    const datetime = document.getElementById('timestamp');
    datetime.innerHTML = `As of ${formattedTimestamp}`;

    // Insert data into the panel
    dataPanel.innerHTML = `
      <div class="bg-green-100 p-2 h-full rounded-lg">
        <p class="font-semibold">Temperature:</p>
        <p>${temperature} °C</p>
      </div>
      <div class="bg-yellow-100 p-2 h-full  rounded-lg">
        <p class="font-semibold">Humidity:</p>
        <p>${humidity} %</p>
      </div>
      <div class="bg-red-100 p-2 h-full rounded-lg">
        <p class="font-semibold">Soil Moisture:</p>
        <p>${soilMoisture} %</p>
      </div>
    `;
  } catch (error) {
    console.error('Error fetching latest data:', error);
    const dataPanel = document.getElementById('data-panel');
    dataPanel.innerHTML = '<p class="text-red-500">Failed to load data</p>';
  }
}

async function fetchAllData() {
  try {
    const response = await axios.get('http://localhost:3010/api/last-entities');
    const data = response.data;
    const container = document.getElementById('data-table');

    if (data.length === 0) {
      container.innerHTML = '<tr><td colspan="4" class="text-center py-2">No data available.</td></tr>';
      return;
    }

    const header = document.createElement('tr');
    header.innerHTML = `
      <td class="border px-4 py-2">Timestamp</td>
      <td class="border px-4 py-2">Temperature</td>
      <td class="border px-4 py-2">Soil Moisture</td>
      <td class="border px-4 py-2">Air Humidity</td>
    `;
    container.appendChild(header);

    data.forEach(entity => {
      const row = document.createElement('tr');
      row.innerHTML = `
        <td class="border px-4 py-2">${new Date(entity.timestamp).toLocaleString()}</td>
        <td class="border px-4 py-2">${entity.temperature}</td>
        <td class="border px-4 py-2">${entity.soilMoisture}</td>
        <td class="border px-4 py-2">${entity.humidity}</td>
      `;
      container.appendChild(row);
    });
  } catch (error) {
    console.error('Error fetching data:', error);
  }
}

window.onload = () => {
  fetchLatestData();
  fetchAllData();
};
