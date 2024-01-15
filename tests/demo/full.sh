#!/usr/bin/env bash

# TALK: build a database from given dataset
bin/gbuild lubm data/lubm/lubm.nt

# TALK: query this database with a SPARQL
bin/gquery lubm data/lubm/lubm_q0.sql

# To see the query content
cat data/lubm/lubm_q0.sql

# To see the delete query
cat data/lubm/lubm_delete.sql

# TALK: remove some triples from the database
bin/gquery lubm data/lubm/lubm_delete.sql

# TALK: run the same query again and see what is different in output
bin/gquery lubm data/lubm/lubm_q0.sql

# To see the insert query
cat data/lubm/lubm_insert.sql

# TALK: reinsert the triples which are removed before
bin/gquery lubm data/lubm/lubm_insert.sql

# TALK: run the same query again and you will see that the triple reinserted appears
bin/gquery lubm data/lubm/lubm_q0.sql

