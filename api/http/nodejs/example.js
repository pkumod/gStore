const GStoreClient = require("./index.js");
const client = new GStoreClient(
	"root",
	"123456",
	"http://127.0.0.1:9001"
);

(async function() {
	const result = await client.query("small","select * where {?s ?p ?o}");
	console.log(JSON.stringify(result,","));
})();
