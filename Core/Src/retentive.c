#include "retentive.h"
#include <string.h>

DirectRetentive_t Retentive_Registry[MAX_RETENTIVE_TAGS];
uint8_t retentive_tag_count = 0;

volatile HAL_StatusTypeDef last_erase_status = HAL_OK;
volatile HAL_StatusTypeDef last_program_status = HAL_OK;
volatile uint32_t flash_error_code = 0;

void Save_As_Retentive(void *var_ptr, uint8_t var_size)
{
    if (retentive_tag_count >= MAX_RETENTIVE_TAGS) return;
    Retentive_Registry[retentive_tag_count].ptr = var_ptr;
    Retentive_Registry[retentive_tag_count].size = var_size;
    retentive_tag_count++;
}

void PLC_Retentive_Save(void)
{
    uint8_t buf[RETENTIVE_BUF_SIZE] = {0};
    uint16_t idx = 0;

    for (int i = 0; i < retentive_tag_count; i++) {
        memcpy(&buf[idx], Retentive_Registry[i].ptr, Retentive_Registry[i].size);
        idx += Retentive_Registry[i].size;
    }

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef e;
    e.TypeErase = FLASH_TYPEERASE_SECTORS;
    e.Sector = FLASH_SECTOR_11;   /* 0x080E0000 */
    e.NbSectors = 1;
    e.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    last_erase_status = HAL_FLASHEx_Erase(&e, &flash_error_code);

    if (last_erase_status == HAL_OK) {
        for (uint32_t i = 0; i < ((idx + 3) / 4); i++) {
            last_program_status = HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_WORD,
                FLASH_RETENTIVE_ADDR + (i * 4),
                ((uint32_t *)buf)[i]);
            if (last_program_status != HAL_OK) {
                break;
            }
        }
    }

    HAL_FLASH_Lock();
}

void PLC_Retentive_Load(void)
{
    if (*((uint32_t *)FLASH_RETENTIVE_ADDR) == 0xFFFFFFFF) return; /* nothing saved yet */

    uint8_t buf[RETENTIVE_BUF_SIZE];
    memcpy(buf, (const void *)FLASH_RETENTIVE_ADDR, RETENTIVE_BUF_SIZE);

    uint16_t idx = 0;
    for (int i = 0; i < retentive_tag_count; i++) {
        memcpy(Retentive_Registry[i].ptr, &buf[idx], Retentive_Registry[i].size);
        idx += Retentive_Registry[i].size;
    }
}
