URL rules:  operation, db_name, ds_path, format, sparql

operation: build, load, unload, query, monitor
db_name: the name of database, like lubm
format: html, json, txt, csv
sparql: select ?s where { ?s ?p ?o . }
ds_path in the server: like /home/data/test.n3

to build a database from a dataset:
http://localhost:9000/?operation=build&db_name=[db_name]&ds_path=[ds_path]

to load a database:
http://localhost:9000/?operation=load&db_name=[db_name]

to query a database:
http://localhost:9000/?operation=query&format=[format]&sparql=[sparql]

to unload a database:
http://localhost:9000/?operation=unload&db_name=[db_name]

to monitor the server:
http://localhost:9000/?operation=monitor

