require('dotenv').config();
const express = require('express');
const path = require('path');
const { TableClient, AzureNamedKeyCredential } = require('@azure/data-tables');
const cors = require('cors');

const app = express();
const port = 3010;

app.use(cors());

const account = process.env.ACCOUNT_NAME;
const accountKey = process.env.ACCOUNT_KEY;
const tableName = process.env.TABLE_NAME;

const credential = new AzureNamedKeyCredential(account, accountKey);
const tableClient = new TableClient(`https://${account}.table.core.windows.net`, tableName, credential);

function serializeEntity(entity) {
  const serialized = {};
  for (const key in entity) {
    if (typeof entity[key] === 'bigint') {
      serialized[key] = entity[key].toString();
    } else {
      serialized[key] = entity[key];
    }
  }
  return serialized;
}

app.get('/api/last-entities', async (req, res) => {
  try {
    const entities = tableClient.listEntities();
    const results = [];
    for await (const entity of entities) {
      results.push(serializeEntity(entity));
    }
    const sortedResults = results.sort((a, b) => new Date(b.Timestamp) - new Date(a.Timestamp)).slice(-20).reverse();
    res.json(sortedResults);
  } catch (error) {
    res.status(500).send(error.message);
  }
});

app.get('/api/last', async (req, res) => {
  try {
    const entities = tableClient.listEntities();
    const results = [];
    for await (const entity of entities) {
      results.push(serializeEntity(entity));
    }
    const sortedResults = results.sort((a, b) => new Date(b.Timestamp) - new Date(a.Timestamp));
    const lastRecord = sortedResults.length > 0 ? sortedResults[sortedResults.length - 1] : null;

    if (lastRecord) {
      res.json(lastRecord);
    } else {
      res.status(404).send('No records found');
    }
  } catch (error) {
    res.status(500).send(error.message);
  }
});

app.listen(port, () => {
  console.log(`Server is running at http://localhost:${port}`);
});
