# gStoreNode

Node.js client for [gStore](https://github.com/Caesar11/gStore)

## Usage

`npm install https://github.com/FrontMage/gStoreNode`

or

`npm install gstore-whu --save`

```javascript
const GStoreClient = require("gstore-whu");
const client = new GStoreClient(
  "username",
  "password",
  "http://localhost:9000"
);

(async function() {
  const result = await client.query("lubm", "select * where {?s ?p ?o}");
  console.log(result);
})();
```
