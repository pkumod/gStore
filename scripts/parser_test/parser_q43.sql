select ?x where
{?x <name> ?name . filter(ucase(str(?name)) = "ALICE")}