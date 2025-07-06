#ifndef CYCAMORE_SRC_CYCAMORE_H_
#define CYCAMORE_SRC_CYCAMORE_H_

// For Doxygen mainpage
/*!
 * \mainpage Cycamore API Reference
 *
 * Welcome to the Cycamore API reference! Below are some helpful links for
 * learning more:
 *   - Cyclus Homepage: https://fuelcycle.org
 *   - GitHub repository: https://github.com/cyclus/cycamore
 *   - Kernel developer guide: https://fuelcycle.org/kernel
 *   - Archetype developer guide: https://fuelcycle.org/arche
 *
 */

// These includes must come before others.
#include "batch_reactor.h"
#include "batch_reactor_tests.h"
#include "cycamore_version.h"
#include "cyclus.h"
#include "deploy_inst.h"
#include "deploy_inst_tests.h"
#include "enrichment.h"
#include "enrichment_tests.h"
#if CYCLUS_HAS_COIN
#include "growth_region.h"
#include "growth_region_tests.h"
#endif
#include "inpro_reactor.h"
#include "inpro_reactor_tests.h"
#include "manager_inst.h"
#include "manager_inst_tests.h"
#include "sink.h"
#include "sink_tests.h"
#include "source.h"
#include "source_tests.h"

#endif  // CYCAMORE_SRC_CYCAMORE_H_
