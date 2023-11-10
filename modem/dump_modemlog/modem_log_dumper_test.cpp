#include "modem_log_dumper.h"

#include <string_view>

#include "android_property_manager.h"
#include "bugreport_constants.h"
#include "dumper.h"
#include "fake_android_property_manager.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "modem_log_constants.h"

namespace pixel_modem::logging {
namespace {

using ::testing::Eq;

inline constexpr static std::string_view kFakePilotLoggingPath =
    "//pilot/logging/path";
inline constexpr static std::string_view kFakeOnDemandLoggingPath =
    "//on/demand/logging/path";
inline constexpr static LogDumpInfo kAlwaysOnLogDumpInfo = {
    kModemAlwaysOnLogDirectory, kBugreportPackingDirectory,
    kDefaultBugreportNumberFiles, kModemLogPrefix};

void StartModemLogging(
    FakeAndroidPropertyManager& fake_android_property_manager) {
  fake_android_property_manager.SetProperty(kModemLoggingEnabledProperty.data(),
                                            kTruthString.data());
}

class MockDumper : public Dumper {
 public:
  ~MockDumper() = default;
  MOCK_METHOD(void, DumpLogs, (const LogDumpInfo&), (override));
  MOCK_METHOD(void, CopyFile, (const FileCopyInfo&), (override));
};

class ModemLogDumperTest : public ::testing::Test {
 protected:
  ModemLogDumperTest()
      : modem_log_dumper(mock_dumper, fake_android_property_manager) {}

  void SetUp() override {
    // set default logging mode to always on logging
    fake_android_property_manager.SetProperty(
        kModemLoggingPathProperty.data(), kModemAlwaysOnLogDirectory.data());
  }

  MockDumper mock_dumper;
  FakeAndroidPropertyManager fake_android_property_manager;
  ModemLogDumper modem_log_dumper;
};

TEST_F(ModemLogDumperTest, DumpLogsDumpsAllDirectoriesAndCopiesAllFiles) {
  EXPECT_CALL(mock_dumper, DumpLogs(Eq(kAlwaysOnLogDumpInfo)));

  for (const LogDumpInfo& log_dump_info : kLogDumpInfo) {
    EXPECT_CALL(mock_dumper, DumpLogs(Eq(log_dump_info)));
  }

  for (const FileCopyInfo& fileCopyInfo : kFileCopyInfo) {
    EXPECT_CALL(mock_dumper, CopyFile(Eq(fileCopyInfo)));
  }

  modem_log_dumper.DumpModemLogs();
}

TEST_F(ModemLogDumperTest, DumpLogsRestartModemLoggingWhenEnabled) {
  StartModemLogging(fake_android_property_manager);

  modem_log_dumper.DumpModemLogs();

  EXPECT_TRUE(fake_android_property_manager.ModemLoggingHasRestarted());
}

TEST_F(ModemLogDumperTest, DumpLogsDoesNotRestartModemLoggingWhenDisabled) {
  modem_log_dumper.DumpModemLogs();

  EXPECT_FALSE(fake_android_property_manager.ModemLoggingHasRestarted());
}

TEST_F(ModemLogDumperTest, DumpLogsDoesNotRestartModemLoggingWhenPilotEnabled) {
  // Enable PILOT
  fake_android_property_manager.SetProperty(kModemLoggingPathProperty.data(),
                                            kFakePilotLoggingPath.data());
  StartModemLogging(fake_android_property_manager);

  modem_log_dumper.DumpModemLogs();

  EXPECT_FALSE(fake_android_property_manager.ModemLoggingHasRestarted());
}

TEST_F(ModemLogDumperTest,
       DumpLogsDoesNotRestartModemLoggingWhenOnDemandLoggingEnabled) {
  // Enable On Demand Logging
  fake_android_property_manager.SetProperty(kModemLoggingPathProperty.data(),
                                            kFakeOnDemandLoggingPath.data());
  StartModemLogging(fake_android_property_manager);

  modem_log_dumper.DumpModemLogs();

  EXPECT_FALSE(fake_android_property_manager.ModemLoggingHasRestarted());
}

}  // namespace
}  // namespace pixel_modem::logging
