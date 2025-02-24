//
// This file is part of the MQT QCEC library released under the MIT license.
// See README.md or go to https://github.com/cda-tum/qcec for more information.
//

#include "EquivalenceCheckingManager.hpp"
#include "checker/dd/applicationscheme/GateCostApplicationScheme.hpp"

#include "gtest/gtest.h"
#include <functional>
#include <sstream>
#include <string>

using namespace dd::literals;

class GateCostApplicationSchemeTest : public testing::Test {
  void SetUp() override {
    dd = std::make_unique<dd::Package<>>(nqubits);
    qc = qc::QuantumComputation(nqubits);
  }

protected:
  dd::QubitCount                 nqubits = 3U;
  std::unique_ptr<dd::Package<>> dd;
  qc::QuantumComputation         qc;
};

TEST_F(GateCostApplicationSchemeTest, SchemeFromProfile) {
  std::string   filename = "simple.profile";
  std::ofstream ofs(filename);

  // create a very simple profile that just specifies that a Toffoli corresponds
  // to 15 gates
  ofs << "x 2 15\n";
  ofs.close();

  // apply Toffoli gate
  qc.x(0, {1_pc, 2_pc});

  auto tm = ec::TaskManager<qc::MatrixDD>(qc, dd);

  auto scheme = ec::GateCostApplicationScheme(tm, tm, filename);

  const auto [left, right] = scheme();

  EXPECT_EQ(left, 1U);
  EXPECT_EQ(right, 15U);

  ec::Configuration config{};
  config.application.profile           = filename;
  config.application.alternatingScheme = ec::ApplicationSchemeType::GateCost;
  ec::EquivalenceCheckingManager ecm(qc, qc, config);
  ecm.run();
  EXPECT_TRUE(ecm.getResults().consideredEquivalent());
  std::cout << ecm.toString() << std::endl;
}

TEST_F(GateCostApplicationSchemeTest, iSWAP) {
  qc.iswap(0, 1);

  auto tm = ec::TaskManager<qc::MatrixDD>(qc, dd);

  auto scheme = ec::GateCostApplicationScheme(tm, tm, &ec::legacyCostFunction);

  const auto [left, right] = scheme();

  EXPECT_EQ(left, 1U);
  EXPECT_EQ(right, 6U);
}

TEST_F(GateCostApplicationSchemeTest, Peres) {
  qc.peres(1, 2, 0_pc);

  auto tm = ec::TaskManager<qc::MatrixDD>(qc, dd);

  auto scheme = ec::GateCostApplicationScheme(tm, tm, &ec::legacyCostFunction);

  const auto [left, right] = scheme();

  EXPECT_EQ(left, 1U);
  EXPECT_EQ(right, 15U);
}
