PREFIX : <http://example/>
SELECT * 
{ 
  ?s ?p ?o 
  MINUS { :a :b :c }
}
