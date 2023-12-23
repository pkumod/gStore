#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause

import os
import shutil
import logging

# configure logging
logging.basicConfig(level=logging.INFO)

# Python built-in archive formats
SUPPORTED_ARCHIVE_EXTS = [".zip", ".tar.gz", ".tar.bz2", ".tar.xz", ".tar", ".tgz", ".tbz2", ".txz"]

def decompress_file(file_path, destination_path, overwrite=True):
    _dir_path = os.path.dirname(destination_path)
    if not os.path.exists(_dir_path):
        os.makedirs(_dir_path)
    
    if os.path.exists(destination_path):
        if overwrite:
            logging.info("%s already exists, removing", destination_path)
            shutil.rmtree(destination_path)
        else:
            logging.info("%s already exists, skipping decompression", destination_path)
            return
        
    for _ext in SUPPORTED_ARCHIVE_EXTS:
        if file_path.endswith(_ext):
            logging.info("Decompressing %s to %s", file_path, destination_path)
            shutil.unpack_archive(file_path, destination_path)

