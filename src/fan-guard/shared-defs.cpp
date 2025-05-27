#include "esp_attr.h"
#include <Arduino.h>
#include "config.h"
#include "shared-defs.h"

QueueHandle_t xQueue_data = NULL;

RTC_DATA_ATTR volatile bool is_first_boot = true;
RTC_DATA_ATTR int id_device = 0;
RTC_DATA_ATTR uint8_t calculated_threshold=0;

const char* dataTypeToString(DataType type) {
  switch (type) {
    case TYPE_INA: return "INA";
    case TYPE_MPU: return "MPU";
    default:  return "unknown";
  }
}

const char* dataClassificationToString(DataClassification classification) {
  switch (classification) {
    case CLASS_NORMAL:    return "normal";
    case CLASS_BEARING: return "bearing";
    case CLASS_OFF:     return "turned_off";
    case CLASS_FLUCTUATIONS:     return "fluctuations";
    case CLASS_OBSTRUCTION: return "obstruction";
    default:                  return "uncertain";
  }
}

DataClassification stringToDataClassification(const char* classificationString) {
  if (classificationString == nullptr) {
    return CLASS_UNKNOWN; // Handle null input
  }
  if (strcmp(classificationString, "normal") == 0) {
    return CLASS_NORMAL;
  } else if (strcmp(classificationString, "bearing") == 0) {
    return CLASS_BEARING;
  } else if (strcmp(classificationString, "turned_off") == 0) {
    return CLASS_OFF;
  } else if (strcmp(classificationString, "fluctuations") == 0) {
    return CLASS_FLUCTUATIONS;
  } else if (strcmp(classificationString, "uncertain") == 0) {
    return CLASS_UNKNOWN; 
  } else if (strcmp(classificationString, "obstruction") == 0) {
    return CLASS_OBSTRUCTION;
  }
  return CLASS_UNKNOWN; 
}


void init_shared_queues() {
    xQueue_data = xQueueCreate(QUEUE_SIZE, sizeof(data_to_send_t));
    if(xQueue_data ==  NULL) {
        Serial.println("Queue creation failed!");
        while(1); // Halt on critical failure
    }
    
}