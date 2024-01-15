PREFIX : <http://example/>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>

SELECT DISTINCT ?s
WHERE {
	?s ?p ?o
	MINUS {
		?s foaf:givenName "Bob" .
		MINUS {
			?s foaf:familyName "Jones" .
		}
	}
}