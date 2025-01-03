#pragma once
#include "../KVstore/KVstore.h"
#include "../Query/Algorithm/CSRQueryHandler.h"

class GAnalysis
{
    private:
    std::shared_ptr<KVstore> kvstore_;
    std::shared_ptr<CSRQueryHandler> csrHandler_;

    public:
    GAnalysis(std::shared_ptr<KVstore> kvstore, std::shared_ptr<CSRQueryHandler> csrHandler);
    ~GAnalysis(){}

    public:
    // id to string
    std::string getStringByID(TYPE_ENTITY_LITERAL_ID id)const;
    std::string getEntityByID(TYPE_ENTITY_LITERAL_ID id) const;
    std::string getPredicateByID(TYPE_PREDICATE_ID id) const;

    // string to id
    TYPE_ENTITY_LITERAL_ID getIDByString(std::string str)const;
    TYPE_ENTITY_LITERAL_ID getIDByEntity(std::string entity) const;
    TYPE_PREDICATE_ID getIDByPredicate(std::string predicate) const;

    // csr
	std::shared_ptr<CSRQueryHandler> getCsrHandler();
};