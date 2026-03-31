# gStore Test Help Document
## 1. Prerequisites
You need to complete the compilation of gStore first. For compilation, please refer to the Quick Start section on the project homepage. Ensure that the following command can be executed normally in the gStore directory:
```bash
bin/gconsole -v
# gStore version: 1.4 Source distribution
# Copyright (c) 2016-2026, pkumod and topgraph and/or its affiliates.
```
## 2. Test Scripts
The main gStore test scripts include:
- gtest.sh: Basic function testing, including build, load, query, unload, and delete;
- basic_test.sh: Correctness verification testing, including build, load, delete, import, query, unload, and delete for four datasets: bbug, lubm, num, and small;
- parser_test.sh: Parsing verification testing; the test datasets are located in scripts/test/parser_test;
- update_test.sh: Data update testing, and verification of the correctness of updated data;
- transaction_test.sh: Transaction operation testing, including begin, execute, commit, and rollback, as well as verifying data accuracy;
- api_test.sh: HTTP API interface testing, covering 27 common types of interface tests.
## 3. Test Steps
You can choose to run the full test suite or a specific individual test. The specific test steps are as follows:
**Full Test**
The full test will execute the five scripts mentioned above in sequence and will automatically start and stop the gserver service.
```bash
# Please ensure you are currently in the GSTORE_HOME directory
sh scripts/test/test.sh
```
**Individual Test**
For an individual test, you need to manually start the gserver service first. The startup command is as follows:
```bash
# Please ensure you are currently in the GSTORE_HOME directory
# -b indicates running in the background
bin/gserver -b
# IP black list enabled.
# [==================================================] Load cache done!   
# Server started at port 9000
```
Then execute the specific test script:
```bash
# Please ensure you are currently in the GSTORE_HOME directory
sh scripts/test/basic_test.sh
```
After the test is completed, if you need to stop the gserver service, you can execute the following command:
```bash
# Please ensure you are currently in the GSTORE_HOME directory
bin/gserver -t
# the Server [64543] stop successfully.
```
