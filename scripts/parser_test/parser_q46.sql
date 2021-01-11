select ?x where
{?x <name> ?name . filter(strstarts(lcase(?name), "ali"))}