//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// File names used by DB code

#pragma once
#include <stdint.h>
#include <unordered_map>
#include <string>
#include <vector>

#include "options/db_options.h"
#include "port/port.h"
#include "rocksdb/file_system.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/status.h"
#include "rocksdb/transaction_log.h"

namespace ROCKSDB_NAMESPACE {

class Env;
class Directory;
class SystemClock;
class WritableFileWriter;

#ifdef OS_WIN
constexpr char kFilePathSeparator = '\\';
#else
constexpr char kFilePathSeparator = '/';
#endif

// Some non-sensitive files are not encrypted to preserve atomicity of file
// operations.
extern bool IsCurrentFile(const std::string& fname);

// Determine if the content is read from the valid current file.
extern bool IsValidCurrentFile(
    std::unique_ptr<rocksdb::SequentialFile> seq_file);

// Return the name of the log file with the specified number
// in the db named by "dbname".  The result will be prefixed with
// "dbname".
extern std::string LogFileName(const std::string& dbname, uint64_t number);

extern std::string LogFileName(uint64_t number);

extern std::string BlobFileName(uint64_t number);

extern std::string BlobFileName(const std::string& bdirname, uint64_t number);

extern std::string BlobFileName(const std::string& dbname,
                                const std::string& blob_dir, uint64_t number);

extern std::string ArchivalDirectory(const std::string& dbname);

//  Return the name of the archived log file with the specified number
//  in the db named by "dbname". The result will be prefixed with "dbname".
extern std::string ArchivedLogFileName(const std::string& dbname,
                                       uint64_t num);

extern std::string MakeTableFileName(const std::string& name, uint64_t number);

extern std::string MakeTableFileName(uint64_t number);

// Return the name of sstable with LevelDB suffix
// created from RocksDB sstable suffixed name
extern std::string Rocks2LevelTableFileName(const std::string& fullname);

// the reverse function of MakeTableFileName
// TODO(yhchiang): could merge this function with ParseFileName()
extern uint64_t TableFileNameToNumber(const std::string& name);

// Return the name of the sstable with the specified number
// in the db named by "dbname".  The result will be prefixed with
// "dbname".
extern std::string TableFileName(const std::vector<DbPath>& db_paths,
                                 uint64_t number, uint32_t path_id);

// Sufficient buffer size for FormatFileNumber.
const size_t kFormatFileNumberBufSize = 38;

extern void FormatFileNumber(uint64_t number, uint32_t path_id, char* out_buf,
                             size_t out_buf_size);

// Return the name of the descriptor file for the db named by
// "dbname" and the specified incarnation number.  The result will be
// prefixed with "dbname".
extern std::string DescriptorFileName(const std::string& dbname,
                                      uint64_t number);

extern std::string DescriptorFileName(uint64_t number);

extern const std::string kCurrentFileName;  // = "CURRENT"

// Return the name of the current file.  This file contains the name
// of the current manifest file.  The result will be prefixed with
// "dbname".
extern std::string CurrentFileName(const std::string& dbname);

// Return the name of the lock file for the db named by
// "dbname".  The result will be prefixed with "dbname".
extern std::string LockFileName(const std::string& dbname);

// Return the name of a temporary file owned by the db named "dbname".
// The result will be prefixed with "dbname".
extern std::string TempFileName(const std::string& dbname, uint64_t number);

// A helper structure for prefix of info log names.
struct InfoLogPrefix {
  char buf[260];
  Slice prefix;
  // Prefix with DB absolute path encoded
  explicit InfoLogPrefix(bool has_log_dir, const std::string& db_absolute_path);
  // Default Prefix
  explicit InfoLogPrefix();
};

// Return the name of the info log file for "dbname".
extern std::string InfoLogFileName(const std::string& dbname,
                                   const std::string& db_path = "",
                                   const std::string& log_dir = "");

// Return the name of the old info log file for "dbname".
extern std::string OldInfoLogFileName(const std::string& dbname, uint64_t ts,
                                      const std::string& db_path = "",
                                      const std::string& log_dir = "");

extern const std::string kOptionsFileNamePrefix;  // = "OPTIONS-"
extern const std::string kTempFileNameSuffix;     // = "dbtmp"

// Return a options file name given the "dbname" and file number.
// Format:  OPTIONS-[number].dbtmp
extern std::string OptionsFileName(const std::string& dbname,
                                   uint64_t file_num);
extern std::string OptionsFileName(uint64_t file_num);

// Return a temp options file name given the "dbname" and file number.
// Format:  OPTIONS-[number]
extern std::string TempOptionsFileName(const std::string& dbname,
                                       uint64_t file_num);

// Return the name to use for a metadatabase. The result will be prefixed with
// "dbname".
extern std::string MetaDatabaseName(const std::string& dbname,
                                    uint64_t number);

// Return the name of the Identity file which stores a unique number for the db
// that will get regenerated if the db loses all its data and is recreated fresh
// either from a backup-image or empty
extern std::string IdentityFileName(const std::string& dbname);

// If filename is a rocksdb file, store the type of the file in *type.
// The number encoded in the filename is stored in *number.  If the
// filename was successfully parsed, returns true.  Else return false.
// info_log_name_prefix is the path of info logs.
extern bool ParseFileName(const std::string& filename, uint64_t* number,
                          const Slice& info_log_name_prefix, FileType* type,
                          WalFileType* log_type = nullptr);
// Same as previous function, but skip info log files.
extern bool ParseFileName(const std::string& filename, uint64_t* number,
                          FileType* type, WalFileType* log_type = nullptr);

// Make the CURRENT file point to the descriptor file with the
// specified number.
extern IOStatus SetCurrentFile(FileSystem* fs, const std::string& dbname,
                               uint64_t descriptor_number,
                               FSDirectory* directory_to_fsync);

// Make the IDENTITY file for the db
extern Status SetIdentityFile(Env* env, const std::string& dbname,
                              const std::string& db_id = {});

// Sync manifest file `file`.
extern IOStatus SyncManifest(const ImmutableDBOptions* db_options,
                             WritableFileWriter* file);

// Return list of file names of info logs in `file_names`.
// The list only contains file name. The parent directory name is stored
// in `parent_dir`.
// `db_log_dir` should be the one as in options.db_log_dir
extern Status GetInfoLogFiles(const std::shared_ptr<FileSystem>& fs,
                              const std::string& db_log_dir,
                              const std::string& dbname,
                              std::string* parent_dir,
                              std::vector<std::string>* file_names);

extern std::string NormalizePath(const std::string& path);
}  // namespace ROCKSDB_NAMESPACE
