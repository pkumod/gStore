select ?x where
{?x <birthday> ?y . filter(?y < now())}