/// @file src/umid/data/process_image.c
/// @brief Fail-closed entry points for the first-pass UMID process-image service.
#include "umid/data/process_image.h"

uetd_status_t umid_process_image_init(umid_process_image_t* image, const umid_process_image_config_t* config,
                                      umid_process_point_t* staging_points, uetd_u8_t* staging_payload,
                                      umid_process_point_t* published_points, uetd_u8_t* published_payload) {
    (void)image;
    (void)config;
    (void)staging_points;
    (void)staging_payload;
    (void)published_points;
    (void)published_payload;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_process_image_begin_scan(umid_process_image_t* image, uetd_u32_t expected_generation,
                                            uetd_u64_t started_at_us) {
    (void)image;
    (void)expected_generation;
    (void)started_at_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_process_image_stage_point(umid_process_image_t* image,
                                             const umid_process_point_t* point) {
    (void)image;
    (void)point;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_process_image_commit_scan(umid_process_image_t* image, uetd_u64_t committed_at_us) {
    (void)image;
    (void)committed_at_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_process_image_snapshot(const umid_process_image_t* image, umid_process_scan_t* scan) {
    (void)image;
    (void)scan;
    return UETD_NOT_SUPPORTED;
}