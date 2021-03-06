/**
 * Copyright (c) 2016 DeepCortex GmbH <legal@eventql.io>
 * Authors:
 *   - Paul Asmuth <paul@eventql.io>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License ("the license") as
 * published by the Free Software Foundation, either version 3 of the License,
 * or any later version.
 *
 * In accordance with Section 7(e) of the license, the licensing of the Program
 * under the license does not imply a trademark license. Therefore any rights,
 * title and interest in our trademarks remain entirely with us.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You can be released from the requirements of the license by purchasing a
 * commercial license. Buying such a license is mandatory as soon as you develop
 * commercial activities involving this program without disclosing the source
 * code of your own applications
 */
#ifndef _FNORDMETRIC_MYSQLBACKEND_MYSQLTABLEREF_H
#define _FNORDMETRIC_MYSQLBACKEND_MYSQLTABLEREF_H
#include <memory>
#include <eventql/sql/backends/mysql/mysqlconnection.h>
#include <eventql/sql/backends/tableref.h>

namespace csql {
namespace mysql_backend {

class MySQLTableRef : public TableRef {
public:

  MySQLTableRef(
      std::shared_ptr<MySQLConnection> conn,
      const std::string& table_name);

  int getComputedColumnIndex(const std::string& name) override;
  std::string getColumnName(int index) override;
  void executeScan(TableScan* scan) override;
  std::vector<std::string> columns() override;

protected:
  std::shared_ptr<MySQLConnection> conn_;
  std::string table_name_;
  std::vector<std::string> table_columns_;
  std::vector<std::string*> columns_;
};

}
}
#endif
