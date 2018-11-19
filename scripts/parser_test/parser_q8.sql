PREFIX  dc:  <http://purl.org/dc/elements/1.1/>
PREFIX  ns:  <http://example.org/ns#>
SELECT  ?title ?price
WHERE   { ?x ns:price ?price .
          FILTER (?price < "30.5"^^<http://www.w3.org/2001/XMLSchema#float>)
          ?x dc:title ?title . }
