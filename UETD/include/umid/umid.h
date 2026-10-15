/// @file include/umid/umid.h
/// @brief Convenience umbrella for the UMID public interfaces.

#ifndef UETD_UMID_UMBRELLA_H
#define UETD_UMID_UMBRELLA_H

#include "umid/drivers/current.h"
#include "umid/drivers/encoder.h"
#include "umid/drivers/fpga.h"
#include "umid/drivers/gnss.h"
#include "umid/drivers/imu.h"
#include "umid/drivers/magnetometer.h"
#include "umid/drivers/pressure.h"
#include "umid/drivers/range.h"
#include "umid/drivers/temperature.h"
#include "umid/drivers/voltage.h"
#include "umid/health.h"
#include "umid/log.h"
#include "umid/ringbuf.h"
/* The header is portable; its source binding is built only when FatFS is enabled. */
#include "umid/fatfs.h"

#endif /* UETD_UMID_UMBRELLA_H */
