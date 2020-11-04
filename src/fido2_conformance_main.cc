// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "src/command_state.h"
#include "src/constants.h"
#include "src/device_tracker.h"
#include "src/hid/hid_device.h"
#include "src/parameter_check.h"
#include "src/tests/test_series.h"

DEFINE_string(
    token_path, "",
    "The path to the device on your operating system, usually /dev/hidraw*.");

DEFINE_bool(verbose, false, "Printing debug logs, i.e. transmitted packets.");

// Calling this function first connects to the device and then executes all test
// series listed.
//
// Usage example:
//   ./fido2_conformance --token_path=/dev/hidraw4 --verbose
int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_token_path.empty()) {
    std::cout << "Please add the --token_path flag for one of these devices:"
              << std::endl;
    fido2_tests::hid::PrintFidoDevices();
    exit(0);
  }

  if (FLAGS_token_path == "_") {
    // This magic value is used by the run script for comfort.
    FLAGS_token_path = fido2_tests::hid::FindFirstFidoDevicePath();
    std::cout << "Tested device path: " << FLAGS_token_path << std::endl;
  }

  fido2_tests::DeviceTracker tracker;
  std::unique_ptr<fido2_tests::DeviceInterface> device =
      std::make_unique<fido2_tests::hid::HidDevice>(&tracker, FLAGS_token_path,
                                                    FLAGS_verbose);
  CHECK(fido2_tests::Status::kErrNone == device->Init())
      << "CTAPHID initialization failed";
  device->Wink();
  // Resets and initializes.
  fido2_tests::CommandState command_state(device.get(), &tracker);
  CHECK(tracker.HasOption("rk"))
      << "The test tool expects resident key support.";
  CHECK(tracker.HasOption("up"))
      << "The test tool expects user presence support.";
  CHECK(tracker.HasCborCapability()) << "The test tool expects CBOR support.";

  // Setup and run all tests, while tracking their results.
  fido2_tests::runners::RunTests(device.get(), &tracker, &command_state);
  // Reset the device to a clean state.
  command_state.Reset();

  std::cout << "\nRESULTS" << std::endl;
  tracker.ReportFindings();
  tracker.SaveResultsToFile();
}

