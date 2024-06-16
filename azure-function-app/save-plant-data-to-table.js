module.exports = function (context, IoTHubMessages) {
  context.log(`JavaScript eventhub trigger function called for message array: ${IoTHubMessages}`);
  
  IoTHubMessages.forEach(message => {
      context.log('Message received: ' + JSON.stringify(message));
      var date = Date.now();
      var partitionKey = Math.floor(date / (24 * 60 * 60 * 1000)) + '';
      var rowKey = date + '';
      
      // Parse the JSON message
      var messageObject = JSON.parse(message);
      var temperature = messageObject.temperature;
      var humidity = messageObject.humidity;
      var soilMoisture = messageObject.soilMoisture;
      
      // Store the individual properties in the output table
      context.bindings.outputTable = {
          "partitionKey": partitionKey,
          "rowKey": rowKey,
          "temperature": temperature,
          "humidity": humidity,
          "soilMoisture": soilMoisture,
          "originalMessage": JSON.stringify(message)
      };
  });

  context.done();
};