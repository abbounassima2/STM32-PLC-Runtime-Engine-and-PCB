#ifndef RETENTIVE_H
#define RETENTIVE_H
/*
 * retentive.h
 * -----------
 * Save/restore of tagged variables to internal flash across
 * STOP/power cycles. Registration is currently manual and ad hoc
 * (SET_RETENTIVE() calls in OB100_User) with no size validation at
 * registration time - see analysis Finding #10. Replacing this with
 * a declarative tag table is future work, not done here; this file
 * is a straight extraction of the existing mechanism.
 */

#include <stdint.h>
#include "main.h"          /* HAL_StatusTypeDef */
#include "plc_config.h"

typedef struct {
    void *ptr;
    uint8_t size;
} DirectRetentive_t;

extern DirectRetentive_t Retentive_Registry[MAX_RETENTIVE_TAGS];
extern uint8_t retentive_tag_count;

/* Diagnostics of the last flash operation, useful under Renode/bench. */
extern volatile HAL_StatusTypeDef last_erase_status;
extern volatile HAL_StatusTypeDef last_program_status;
extern volatile uint32_t flash_error_code;

void Save_As_Retentive(void *var_ptr, uint8_t var_size);
#define SET_RETENTIVE(var) Save_As_Retentive(&(var), sizeof(var))

void PLC_Retentive_Save(void);
void PLC_Retentive_Load(void);

#endif /* RETENTIVE_H */
