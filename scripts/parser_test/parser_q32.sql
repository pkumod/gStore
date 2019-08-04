prefix ns:   <http://example.org/ns#>

select ?name where
{
	?x ns:name ?name .
	?x ns:height ?h .
	?x ns:age ?age .
}
order by ?age desc(?h)