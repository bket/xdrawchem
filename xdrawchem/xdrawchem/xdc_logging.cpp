// xdc_logging.cpp — QLoggingCategory definitions for XDrawChem.

#include "xdc_logging.h"

// chemdata / I/O layer
Q_LOGGING_CATEGORY(lcChemData, "xdc.chemdata")
Q_LOGGING_CATEGORY(lcMDL, "xdc.mdl")
Q_LOGGING_CATEGORY(lcCML, "xdc.cml")
Q_LOGGING_CATEGORY(lcXML, "xdc.xml")

// molecule / chemistry layer
Q_LOGGING_CATEGORY(lcMolecule, "xdc.molecule")
Q_LOGGING_CATEGORY(lcSMILES, "xdc.smiles")
Q_LOGGING_CATEGORY(lcSSSR, "xdc.sssr")

// network layer
Q_LOGGING_CATEGORY(lcNetwork, "xdc.network")

// rendering layer
Q_LOGGING_CATEGORY(lcRender, "xdc.render")
