#include <stdint.h>

#define NO_CONTROLLER 0
#define IDE_CONTROLLER 1
#define AHCI_CONTROLLER 2
struct storage_controller_info {
 uint8_t controller_type;
 uint32_t base_1;
 uint32_t base_2;
}__attribute__((packed));
#define MAX_NUMBER_OF_STORAGE_CONTROLLERS 10
struct storage_controller_info storage_controllers[MAX_NUMBER_OF_STORAGE_CONTROLLERS];
uint32_t number_of_storage_controllers;