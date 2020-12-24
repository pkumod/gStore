/*=============================================================================
# Filename: SigEntry.h
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 13:48
# Description: written by liyouhuan and hanshuo
=============================================================================*/

#ifndef _SIGNATURE_SIGENTRY_H
#define _SIGNATURE_SIGENTRY_H

#include "../Util/Util.h"
#include "Signature.h"

class SigEntry {
  private:
  EntitySig sig;
  //-1 if not in leaf node
  TYPE_ENTITY_LITERAL_ID entity_id;

  public:
  SigEntry();
  SigEntry(TYPE_ENTITY_LITERAL_ID _entity_id, EntityBitSet& _bitset);
  SigEntry(const SigEntry& _sig_entry);
  SigEntry(const EntitySig& sig, TYPE_ENTITY_LITERAL_ID _entity_id);
  const EntitySig& getEntitySig() const;
  TYPE_ENTITY_LITERAL_ID getEntityId() const;
  int getSigCount() const;
  SigEntry& operator=(const SigEntry _sig_entry);
  SigEntry& operator|=(const SigEntry _sig_entry);
  bool cover(const SigEntry& _sig_entry) const;
  bool cover(const EntitySig& _sig) const;
  int xEpsilen(const SigEntry& _sig_entry) const;
  int xOR(const SigEntry& _sig_entry) const;
  std::string to_str() const;
};

#endif // _SIGNATURE_SIGENTRY_H
