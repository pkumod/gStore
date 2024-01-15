PREFIX foaf: <http://xmlns.com/foaf/0.1/>
SELECT ?name1 ?name2
WHERE { ?x foaf:name  ?name1 ;
        foaf:mbox  ?mbox1 .
        ?y foaf:name  ?name2 ;
        foaf:mbox  ?mbox2 .
        FILTER (?mbox1 = ?mbox2 && ?name1 != ?name2)
      }
