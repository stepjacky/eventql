/**
 * Copyright (c) 2016 zScale Technology GmbH <legal@zscale.io>
 * Authors:
 *   - Paul Asmuth <paul@zscale.io>
 *   - Laura Schlimmer <laura@zscale.io>
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
#include <eventql/cli/commands/rebalance.h>
#include <eventql/util/cli/flagparser.h>
#include "eventql/config/config_directory.h"
#include "eventql/master/master_service.h"

namespace eventql {
namespace cli {

const String Rebalance::kName_ = "rebalance";
const String Rebalance::kDescription_ = "Rebalance.";

Rebalance::Rebalance(
    RefPtr<ProcessConfig> process_cfg) :
    process_cfg_(process_cfg) {}

Status Rebalance::execute(
    const std::vector<std::string>& argv,
    FileInputStream* stdin_is,
    OutputStream* stdout_os,
    OutputStream* stderr_os) {
  auto zookeeper_addr = process_cfg_->getString("evqlctl", "zookeeper_addr");
  if (zookeeper_addr.isEmpty()) {
    stderr_os->write("ERROR: zookeeper address not specified\n");
    return Status(eFlagError);
  }

  ::cli::FlagParser flags;
  flags.defineFlag(
      "cluster_name",
      ::cli::FlagParser::T_STRING,
      true,
      NULL,
      NULL,
      "node name",
      "<string>");

  try {
    flags.parseArgv(argv);

    ScopedPtr<ConfigDirectory> cdir;
    {
      auto rc = ConfigDirectoryFactory::getConfigDirectoryForClient(
          process_cfg_.get(),
          &cdir);

      if (rc.isSuccess()) {
        rc = cdir->start();
      }

      if (!rc.isSuccess()) {
        stderr_os->write(StringUtil::format("ERROR: $0\n", rc.message()));
        return rc;
      }
    }

    MasterService master(cdir.get());
    {
      auto rc = master.runOnce();
      cdir->stop();

      if (!rc.isSuccess()) {
        stderr_os->write(StringUtil::format("ERROR: $0\n", rc.message()));
        return rc;
      } else {
        stdout_os->write("SUCCESS");
      }
    }

  } catch (const Exception& e) {
    stderr_os->write(StringUtil::format(
        "$0: $1\n",
        e.getTypeName(),
        e.getMessage()));
    return Status(e);
  }

  return Status::success();
}

const String& Rebalance::getName() const {
  return kName_;
}

const String& Rebalance::getDescription() const {
  return kDescription_;
}

void Rebalance::printHelp(OutputStream* stdout_os) const {
  stdout_os->write(StringUtil::format(
      "\nevqlctl-$0 - $1\n\n", kName_, kDescription_));

  stdout_os->write(
      "Usage: evqlctl [OPTIONS]\n"
      "  --cluster_name <node name>       The name of the cluster.\n"
  );
}

} // namespace cli
} // namespace eventql



