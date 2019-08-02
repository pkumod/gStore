prefix ns:   <http://example.org/ns#>

select ?name ?h where
{
	?x ns:name ?name .
	?x ns:height ?h .
}
order by desc(?h)