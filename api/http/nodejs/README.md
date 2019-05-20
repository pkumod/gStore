# gStoreNode

Node.js client for [gStore](https://github.com/pkumod/gStore)

## Usage

```javascript
const GstoreConnector = require("GstoreConnector.js");
const gc = new GstoreConnector(
  "localhost",
  9000,
  "root",
  "123456"
);

(async function() {
  const res = await gc.query("lubm", "json", "select * where {?s ?p ?o}");
  console.log(JSON.stringify(res,","));
})();
```
