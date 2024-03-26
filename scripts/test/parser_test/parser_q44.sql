select ?x where
{?x <name> ?name . filter(str(ucase(?name)) = "ALICE")}