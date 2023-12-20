#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause

import os
import zipfile
import shutil
import logging
import bz2

# configure logging
logging.basicConfig(level=logging.INFO)

SUPPORTED_ARCHIVE_EXTS = [".zip", ".tar.gz", ".tar.bz2", ".tar.xz", ".tar", ".tgz", ".tbz2", ".txz"]

def decompress_file(file_path, destination_path):
    _dir_path = os.path.dirname(destination_path)
    if not os.path.exists(_dir_path):
        os.makedirs(_dir_path)
    
    if os.path.exists(destination_path):
        logging.info("File %s already exists, skipping decompression", destination_path)
        return
    
    for _ext in SUPPORTED_ARCHIVE_EXTS:
        if file_path.endswith(_ext):
            logging.info("Decompressing %s to %s", file_path, destination_path)
            shutil.unpack_archive(file_path, destination_path)

    