/// @file include/uef/umid/umid.h
/// @brief Convenience umbrella for the UMID public interfaces.

#ifndef UEF_UMID_UMBRELLA_H
#define UEF_UMID_UMBRELLA_H

#include "uef/umid/umid_imu.h"
#include "uef/umid/umid_encoder.h"
#include "uef/umid/umid_current.h"
#include "uef/umid/umid_voltage.h"
#include "uef/umid/umid_pressure.h"
#include "uef/umid/umid_temperature.h"
#include "uef/umid/umid_magnetometer.h"
#include "uef/umid/umid_range.h"
#include "uef/umid/umid_gnss.h"
#include "uef/umid/umid_ringbuf.h"
#include "uef/umid/umid_log.h"
#include "uef/umid/umid_health.h"
/* The header is portable; its source binding is built only when FatFS is enabled. */
#include "uef/umid/umid_fatfs.h"

#endif /* UEF_UMID_UMBRELLA_H */
