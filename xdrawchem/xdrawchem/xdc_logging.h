// xdc_logging.h — centralized QLoggingCategory definitions for XDrawChem.
//
// Usage:
//   #include "xdc_logging.h"
//   qCDebug(lcChemData) << "message";
//   qCWarning(lcMDL) << "warning";
//
// To silence at runtime:
//   QT_LOGGING_RULES="xdc.*=false" ./xdrawchem

#pragma once

#include <QLoggingCategory>

// chemdata / I/O layer
Q_DECLARE_LOGGING_CATEGORY(lcChemData)
Q_DECLARE_LOGGING_CATEGORY(lcMDL)
Q_DECLARE_LOGGING_CATEGORY(lcCML)
Q_DECLARE_LOGGING_CATEGORY(lcXML)

// molecule / chemistry layer
Q_DECLARE_LOGGING_CATEGORY(lcMolecule)
Q_DECLARE_LOGGING_CATEGORY(lcSMILES)
Q_DECLARE_LOGGING_CATEGORY(lcSSSR)

// network layer
Q_DECLARE_LOGGING_CATEGORY(lcNetwork)

// rendering layer
Q_DECLARE_LOGGING_CATEGORY(lcRender)
