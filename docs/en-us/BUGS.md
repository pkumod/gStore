**This file maintains details of the bugs not solved currently.**

---

#### BUG_StringIndex_pread

StringIndex::randomAcces()

StringIndex::trySequenceAccess()

when we insert a triple via ghttp, and query this triple immediately, we will find that answer is wrong.
when we run this query for several times, each time we will get a different answer.
Sometimes, we will get messy code.
With the same reason, if we use bin/gquery db to enter the gquery console, insert and query within this console, we will get similar errors.
Amazingly, if we quit the console and restart, run this query again, we will get the correct answer!

The problem appears after we replace fread in StringIndex with pread, to support conncurrent queries.
The inherent reason have not been found now.
As a result, we change it back to fread, and use a lock for the StringIndex to block concurrent reads.
This is not supposed to cause a great loss in performance, because all operations to a single disk will be executed sequentially by the disk controller.

---

