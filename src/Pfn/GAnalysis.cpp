#include "GAnalysis.h"

GAnalysis::GAnalysis(std::shared_ptr<KVstore> kvstore, std::shared_ptr<CSRQueryHandler> csrHandler)
{
    kvstore_ = kvstore;
    csrHandler_ = csrHandler;
}

std::shared_ptr<CSRQueryHandler> GAnalysis::getCsrHandler()
{
    return csrHandler_;
}

std::string GAnalysis::getStringByID(TYPE_ENTITY_LITERAL_ID id)const
{
    return kvstore_->getStringByID(id);
}

std::string GAnalysis::getEntityByID(TYPE_ENTITY_LITERAL_ID id) const
{
    return kvstore_->getEntityByID(id);
}

 std::string GAnalysis::getPredicateByID(TYPE_PREDICATE_ID id) const
 {
    return kvstore_->getPredicateByID(id);
 }

TYPE_ENTITY_LITERAL_ID GAnalysis::getIDByString(std::string str)const
{
    return kvstore_->getIDByString(str);
}

TYPE_ENTITY_LITERAL_ID GAnalysis::getIDByEntity(std::string entity) const
{
    return kvstore_->getIDByString(entity);
}

TYPE_PREDICATE_ID GAnalysis::getIDByPredicate(std::string predicate) const
{
    return kvstore_->getIDByPredicate(predicate);
}
